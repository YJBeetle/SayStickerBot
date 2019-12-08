#include "InlineQuery.h"

#include "Log.h"
#include "Global.h"
#include "UsersData.h"
#include "StringCheck.h"

using namespace std;
using namespace TgBot;

void pushEmpty(const Api &api,
               vector<InlineQueryResult::Ptr> &results)
{
    auto result = make_shared<InlineQueryResultArticle>();

    result->title = "没搜索到信息";
    result->id = "nouser";

    auto text = make_shared<InputTextMessageContent>();
    text->messageText = "这里是贴图语录机器人，将消息转发给我以创建贴图。";
    result->inputMessageContent = text;

    InlineKeyboardButton::Ptr button = make_shared<InlineKeyboardButton>();
    button->text = "Start";
    button->switchInlineQuery = "";
    auto replyMarkup = make_shared<InlineKeyboardMarkup>();
    replyMarkup->inlineKeyboard.resize(1);
    replyMarkup->inlineKeyboard[0].resize(1);
    replyMarkup->inlineKeyboard[0][0] = button;
    result->replyMarkup = replyMarkup;

    results.push_back(result);
}

void pushStickerByUsernameFuzzy(const Api &api,
                                vector<InlineQueryResult::Ptr> &results,
                                const string &usernameKey)
{
    auto ret = usersData->searchByUsernameFuzzy(usernameKey);
    for (auto c : ret)
    {
        LogV("%d %s", c.id, c.fileId.c_str());
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = c.id;
        result->stickerFileId = c.fileId;
        results.push_back(result);
    }
}

void pushStickerByUsernameAndContentFuzzy(const Api &api,
                                          vector<InlineQueryResult::Ptr> &results,
                                          const string &username,
                                          const string &contentKey)
{
    auto ret = usersData->searchByUsernameAndContentFuzzy(username, contentKey);
    for (auto c : ret)
    {
        LogV("%d %s", c.id, c.fileId.c_str());
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = c.id;
        result->stickerFileId = c.fileId;
        results.push_back(result);
    }
}

void pushStickerByContentFuzzy(const Api &api,
                               vector<InlineQueryResult::Ptr> &results,
                               const string &contentKey)
{
    auto ret = usersData->searchByContentFuzzy(contentKey);
    for (auto c : ret)
    {
        LogV("%d %s", c.id, c.fileId.c_str());
        auto result = make_shared<InlineQueryResultCachedSticker>();
        result->id = c.id;
        result->stickerFileId = c.fileId;
        results.push_back(result);
    }
}

void pushStickerOnInlineQuery(const Api &api,
                              vector<InlineQueryResult::Ptr> &results,
                              const string &query)
{
    if (query.c_str()[0] == '@')
    {
        auto p = query.find(' ');
        if (p == string::npos)
            pushStickerByUsernameFuzzy(api, results, query.c_str() + 1);
        else
            pushStickerByUsernameAndContentFuzzy(api, results, query.substr(0, p), query.substr(p));
    }
    else
        pushStickerByContentFuzzy(api, results, query);

    // if (results.size() == 0) // 如果列表依然是空的，则显示按钮用于创建
    //     pushEmpty(api, results);
}
