#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <sqlite3.h>

#define USERDATAFILEPATH "UsersData.txt"

class UsersData
{
private:
    sqlite3 *db;

    sqlite3_stmt *stmtAdd;
    sqlite3_stmt *stmtRemove;
    sqlite3_stmt *stmtSearchByUsername;
    sqlite3_stmt *stmtSearchByUsernameFuzzy;
    sqlite3_stmt *stmtSearchByUserIdAndContent;

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

    std::vector<Column> searchByUsername(const std::string &username);
    std::vector<Column> searchByUsernameFuzzy(const std::string &usernameKey);
    std::vector<Column> searchByUserIdAndContent(int userId, const std::string &content);
};
