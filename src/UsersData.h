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

public:
    UsersData(const std::string &dbFile = "UsersData.db");
    ~UsersData();

    void add( int fromUserId, const std::string &fromUsername, const std::string &content, const std::string &fileId);
    void remove(int id);
    std::vector<std::string> searchByUsernameFuzzy( const std::string &__username);
};
