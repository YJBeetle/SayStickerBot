#include "UsersData.h"

#include <iostream>

#include "Log.h"
#include "Global.h"
#include "StringCheck.h"

using namespace std;
using namespace TgBot;

UsersData::UsersData(const std::string &dbFile)
{
    int rc;
    rc = sqlite3_open(dbFile.c_str(), &db);
    if (rc)
    {
        sqlite3_close(db);
        throw runtime_error("Can't open database");
    }

    // 初始化表
    // DROP TABLE "messages"; // 删表
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, R"(CREATE TABLE "messages" ("id" integer, "fromUserId" integer, "fromUsername" text, "content" text, "fileId" text, PRIMARY KEY (id));)", -1, &stmt, NULL);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR)
    {
        sqlite3_close(db);
        throw runtime_error("Create table error");
    }
    sqlite3_finalize(stmt);

    // 预编译一些常用的
    sqlite3_prepare_v2(db, R"(INSERT INTO  "messages" ("fromUserId", "fromUsername", "content", "fileId") VALUES (?, ?, ?, ?);)", -1, &stmtAdd, NULL);
    sqlite3_prepare_v2(db, R"(DELETE FROM "messages" WHERE "id" = ?;)", -1, &stmtRemove, NULL);
    sqlite3_prepare_v2(db, R"(SELECT "id", "fromUserId", "fromUsername", "content", "fileId" FROM "messages" WHERE "fromUsername" = ? COLLATE NOCASE ORDER BY "id" DESC LIMIT 20 OFFSET 0;)", -1, &stmtSearchByUsername, NULL);
    sqlite3_prepare_v2(db, R"(SELECT "id", "fromUserId", "fromUsername", "content", "fileId" FROM "messages" WHERE "fromUsername" LIKE ? COLLATE NOCASE ORDER BY "id" DESC LIMIT 20 OFFSET 0;)", -1, &stmtSearchByUsernameFuzzy, NULL);
}

UsersData::~UsersData()
{
    sqlite3_finalize(stmtAdd);
    sqlite3_finalize(stmtRemove);
    sqlite3_finalize(stmtSearchByUsername);
    sqlite3_finalize(stmtSearchByUsernameFuzzy);
    sqlite3_close(db);
}

void UsersData::add(int fromUserId, const string &fromUsername, const string &content, const string &fileId)
{
    sqlite3_bind_int(stmtAdd, 1, fromUserId);
    sqlite3_bind_text(stmtAdd, 2, fromUsername.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmtAdd, 3, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmtAdd, 4, fileId.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmtAdd);
    if (SQLITE_DONE != rc)
    {
        throw runtime_error("add error");
    }
    sqlite3_reset(stmtAdd);
}

void UsersData::remove(int id)
{
    sqlite3_bind_int(stmtRemove, 1, id);
    int rc = sqlite3_step(stmtRemove);
    if (SQLITE_DONE != rc)
    {
        throw runtime_error("remove error");
    }
    sqlite3_reset(stmtRemove);
}

vector<UsersData::Column> UsersData::searchByUsername(const string &username)
{
    vector<Column> ret;

    sqlite3_bind_text(stmtSearchByUsername, 1, username.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmtSearchByUsername) == SQLITE_ROW)
    {
        Column column;
        column.id = sqlite3_column_int(stmtSearchByUsername, 0);
        column.fromUserId = sqlite3_column_int(stmtSearchByUsername, 1);
        column.fromUsername = (const char *)sqlite3_column_text(stmtSearchByUsername, 2);
        column.content = (const char *)sqlite3_column_text(stmtSearchByUsername, 3);
        column.fileId = (const char *)sqlite3_column_text(stmtSearchByUsername, 4);
        ret.push_back(column);
    }
    sqlite3_reset(stmtSearchByUsername);

    return ret;
}

vector<UsersData::Column> UsersData::searchByUsernameFuzzy(const string &usernameKey)
{
    vector<Column> ret;

    sqlite3_bind_text(stmtSearchByUsernameFuzzy, 1, ("%" + usernameKey + "%").c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmtSearchByUsernameFuzzy) == SQLITE_ROW)
    {
        Column column;
        column.id = sqlite3_column_int(stmtSearchByUsernameFuzzy, 0);
        column.fromUserId = sqlite3_column_int(stmtSearchByUsernameFuzzy, 1);
        column.fromUsername = (const char *)sqlite3_column_text(stmtSearchByUsernameFuzzy, 2);
        column.content = (const char *)sqlite3_column_text(stmtSearchByUsernameFuzzy, 3);
        column.fileId = (const char *)sqlite3_column_text(stmtSearchByUsernameFuzzy, 4);
        ret.push_back(column);
    }
    sqlite3_reset(stmtSearchByUsernameFuzzy);

    return ret;
}
