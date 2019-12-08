#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <tgbot/tgbot.h>
#include <sqlite3.h>

#define USERDATAFILEPATH "UsersData.txt"

class UsersData
{
private:
    sqlite3 *db;
    // std::unordered_map<std::string, std::string> data;

public:
    UsersData(const std::string &dbFile = "UsersData.db");
    ~UsersData();

    void add(const std::string &user, const std::string &fileId);
    void remove(const std::string &user, const std::string &fileId);
    std::vector<std::string> searchByUsername(const TgBot::Api &api, const std::string &__username);
};
