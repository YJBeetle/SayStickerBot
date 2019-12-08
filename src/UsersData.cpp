#include "UsersData.h"

#include "Log.h"
#include "Global.h"
#include "StringCheck.h"

using namespace std;
using namespace TgBot;

UsersData::UsersData(const std::string &dbFile)
{
    int rc = sqlite3_open(dbFile.c_str(), &db);
    if (rc)
    {
        sqlite3_close(db);
        throw runtime_error("Can't open database");
    }
}

UsersData::~UsersData()
{
    sqlite3_close(db);
}

void UsersData::add(const string &user, const string &fileId)
{
    ;
}

void UsersData::remove(const string &user, const string &fileId)
{
    ;
}

vector<string> UsersData::searchByUsername(const Api &api, const string &__username)
{
    string username = __username;
    fixUsername(username);
    lowercase(username);

    vector<string> ret;

    if (!checkSelf(username)) // 不允许丢自己
        return ret;

    // auto s = data.find(username);
    // if (s != data.end())
    // {
    //     ret.push_back(s->second);
    // }

    return ret;
}
