#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <sqlite3.h>

#define USERDATAFILEPATH "UsersData.txt"

class UsersData
{
private:
    sqlite3 *db;

    sqlite3_stmt *stmtAdd;
    sqlite3_stmt *stmtRemove;
    sqlite3_stmt *stmtSearchById;
    sqlite3_stmt *stmtSearchByUsername;
    sqlite3_stmt *stmtSearchByUsernameFuzzy;
    sqlite3_stmt *stmtSearchByUsernameAndContentFuzzy;
    sqlite3_stmt *stmtSearchByUserId;
    sqlite3_stmt *stmtSearchByUserIdAndContent;
    sqlite3_stmt *stmtSearchByContentFuzzy;
    sqlite3_stmt *stmtRemoveByUserId;
    sqlite3_stmt *stmtOptOutByUserId;
    sqlite3_stmt *stmtSearchOptOutByUserId;

public:
    struct Column
    {
        int id;
        int fromUserId;
        std::string fromUsername;
        std::string content;
        std::string fileId;
    };

    UsersData(const std::string &dbFile = "UsersData.db");
    ~UsersData();

    void add(int fromUserId, const std::string &fromUsername, const std::string &content, const std::string &fileId);
    void remove(int id);

    std::vector<Column> searchById(int id);
    std::vector<Column> searchByUsername(const std::string &username);
    std::vector<Column> searchByUsernameFuzzy(const std::string &usernameKey);
    std::vector<Column> searchByUsernameAndContentFuzzy(const std::string &username, const std::string &contentKey);
    std::vector<Column> searchByUserId(int userId);
    std::vector<Column> searchByUserIdAndContent(int userId, const std::string &content);
    std::vector<Column> searchByContentFuzzy(const std::string &contentKey);
    void removeByUserId(int userId);
    void optOutByUserId(int userId, const std::string &username);
    bool searchOptOutByUserIdOrUsername(int userId, const std::string &username);
};
