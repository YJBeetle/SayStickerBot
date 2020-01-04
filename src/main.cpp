#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <tgbot/tgbot.h>
#include <ArtRobot/ArtRobot.h>

#include "Global.h"
#include "Log.h"
#include "UsersData.h"
#include "MakeSticker.h"
#include "InlineQuery.h"
#include "Tg.h"
#include "StringCheck.h"

using namespace std;
using namespace cv;
using namespace TgBot;

std::string botUsername;
std::string botUsernameLowercase;
int32_t botId = 0;
UsersData *usersData;

int main()
{
    cout << "====================" << endl
         << "|  SayStickerBot!  |" << endl
         << "====================" << endl;

    usersData = new UsersData();

    string token = getenv("TOKEN");
    Bot bot(token);

    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) { // 处理收到的直接消息
        LogI("Message: \n\tchat: <%s>(%lld)\n\tfrom: <%s>(%d)\n%s", message->chat->username.c_str(), message->chat->id, message->from->username.c_str(), message->from->id, message->text.c_str());

        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->forwardDate) // 是转发的消息
        {
            if (message->forwardFrom)
            {
                string &fromUsername = message->forwardFrom->username;
                int fromUserId = message->forwardFrom->id;
                string &content = message->text;
                LogV("Forward message: \n\tforwardFrom: <%s>(%d)",
                     fromUsername.c_str(),
                     fromUserId);

                if (fromUserId == botId)
                {
                    sendMessage(api, chatId, "哼～ (┙>∧<)┙彡 ┻━┻"); // 禁止收录自己
                    return;
                }

                if (content.empty())
                {
                    sendMessage(api, chatId, "大概是不支持的内容呢。"); // 空消息判断
                    return;
                }

                auto ret = usersData->searchByUserIdAndContent(fromUserId, content); // 搜索是否已经存在
                if (ret.size() == 0)
                { // 不存在
                    auto stickerFileId = MakeSticker(api, chatId, message->forwardFrom, content, message->from->id);
                    if (stickerFileId.length())
                    { // 成功
                        usersData->add(message->forwardFrom->id, message->forwardFrom->username, message->text, stickerFileId);
                        sendSticker(api, chatId, stickerFileId);
                    }
                }
                else
                {                                            // 存在
                    sendSticker(api, chatId, ret[0].fileId); // 直接返回
                }
            }
            else
            { // 被转发用户的隐私设置原因无法获取uid
                sendMessage(api, chatId, "该用户的隐私设置不允许转发。");
            }
            return;
        }

        if (message->text.c_str()[0] == '/') // 如果是指令则跳过
            return;

        if (message->chat->type == Chat::Type::Private) // 只有私聊显示help
            sendMessage(api, chatId, "如果想要什么帮助的话请给我发 /help");
    });

    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) { // /help
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            stringstream ss;
            ss << "这里是贴纸语录机器人。" << endl
               << "与其他的语录机器人类似，这里可以记录您自己或者别人说过的话。" << endl
               << "然而不同的是，我将会把你转发的消息用Sticker的形式呈现出来。" << endl
               << endl
               << "使用我的方法很简单，将您想要保存的消息转发给我即可。" << endl
               << "当然，和大多数情况一样，如果被转发者的Forwarded privacy设置禁止链接到原用户，那么我将不知道您转发的消息来自于何人，故无法收录，这是Telegram的机制所致。" << endl
               << endl
               << "如果需要管理您自己的语录，请给我发送 /list 来列出您已收录的语录。" << endl
               << "在列出的同时，每一句语录前将会有一个序号，如果您需要删除某一行语录，则给我发送 /delete 加您想删除的语录序号即可。" << endl
               << endl
               << "另外需要提醒一点的是，不是每个人都愿意自己的语录在自己不知情的情况下被收录，所以虽然在这里没有任何的限制，但是请在转发消息给我之前得到原消息人的同意/默许。" << endl
               << endl
               << "目前想说的就是这么多，祝使用愉快。" << endl;
            sendMessage(api, chatId, ss.str());
        } // 不支持群组
    });

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) { // /start
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        sendMessage(api, chatId, "欢迎使用贴纸语录机器人。\n如果想要什么帮助的话请给我发 /help");
    });

    bot.getEvents().onCommand("list", [&bot](Message::Ptr message) { // /list
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        auto &command = message->text;

        string retStr;
        if (command == "/list" ||                       // "/list"
            command == "/list@" + botUsernameLowercase) // "/list@SayStkBot"
        {
            auto ret = usersData->searchByUserId(message->from->id);
            stringstream ss;
            for (auto c : ret)
            {
                ss << c.id << " " << c.content << "\n";
            }
            retStr = ss.str();
        }
        else if (StringTools::startsWith(command, "/list ") || // "/list <Username>"
                 StringTools::startsWith(command, "/list@"))   // "/list@<Username>", "/list@SayStkBot <Username>"
        {
            string username;
            if (StringTools::startsWith(command, "/list@" + botUsernameLowercase + " ") || // "/list@SayStkBot <Username>"
                StringTools::startsWith(command, "/list@" + botUsernameLowercase + "@"))   // "/list@SayStkBot@<Username>"
                username = message->text.c_str() + 6 /* strlen("/list@") */ + botUsernameLowercase.length() + 1 /* strlen( ) || strlen(@) */;
            else if (StringTools::startsWith(command, "/list @" + botUsernameLowercase + " ") || // "/list @SayStkBot <Username>"
                     StringTools::startsWith(command, "/list @" + botUsernameLowercase + "@"))   // "/list @SayStkBot@<Username>"
                username = message->text.c_str() + 7 /* strlen("/list @") */ + botUsernameLowercase.length() + 1 /* strlen( ) || strlen(@) */;
            else
                username = message->text.c_str() + 6 /* strlen("/list ") || strlen("/list@") */;

            auto ret = usersData->searchByUsername(username);
            stringstream ss;
            for (auto c : ret)
            {
                ss << c.id << " " << c.content << "\n";
            }
            retStr = ss.str();
        }

        if (retStr.length() == 0)
        {
            retStr = "没有内容.";
        }

        sendMessage(api, chatId, retStr);
    });

    bot.getEvents().onCommand("delete", [&bot](Message::Ptr message) { // /delete
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        auto &command = message->text;

        if (StringTools::startsWith(command, "/delete ")) // "/delete <id>"
        {
            int id = 0;

            try
            {
                id = std::stoi(command.substr(8));
            }
            catch (const std::exception &e)
            {
                LogE("TgBot::Api::deleteStickerFromSet: %s", e.what());
                sendMessage(api, chatId, "id必须为数字");
            }

            if (id)
            {
                auto ret = usersData->searchById(id);
                if (ret.size() && ret[0].fromUserId == message->from->id)
                {
                    usersData->remove(id);
                    try
                    {
                        api.deleteStickerFromSet(ret[0].fileId); // 从tg服务器删除
                    }
                    catch (TgException &e)
                    {
                        LogE("TgBot::Api::deleteStickerFromSet: %s", e.what());
                    }
                    sendMessage(api, chatId, "已删除");
                }
                else
                {
                    sendMessage(api, chatId, "您只能管理自己的消息");
                }
            }
        }
        else
        {
            sendMessage(api, chatId, "语法： /delete id");
        }
    });

    bot.getEvents().onCommand("updateall", [&bot](Message::Ptr message) { // /updateall
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        // TODO:此命令更新本用户的全部贴图 重新上传 更新头像
    });

    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) { // 未知指令
        auto &api = bot.getApi();
        auto chatId = message->chat->id;

        if (message->chat->type == Chat::Type::Private)
        { // 私聊
            sendMessage(api, chatId, "如果想要什么帮助的话请给我发 /help");
        }
    });

    bot.getEvents().onInlineQuery([&bot](InlineQuery::Ptr inlineQuery) {
        LogI("InlineQuery: \n\tfrom: <%s>(%d)\n%s", inlineQuery->from->username.c_str(), inlineQuery->from->id, inlineQuery->query.c_str());

        auto &query = inlineQuery->query;

        vector<InlineQueryResult::Ptr> results; // 准备results

        pushStickerOnInlineQuery(bot.getApi(), results, query);

        // debug json
        // TgTypeParser tgTypeParser;
        // cout << tgTypeParser.parseArray<InlineQueryResult>(&TgTypeParser::parseInlineQueryResult, results) << endl;

        try
        {
            bot.getApi().answerInlineQuery(inlineQuery->id, results);
        }
        catch (TgException &e)
        {
            LogE("answerInlineQuery: %s", e.what());
        }
    });

    while (true)
    {
        try
        {
            LogI("Starting ...");
            botUsernameLowercase = botUsername = bot.getApi().getMe()->username;
            lowercase(botUsernameLowercase);
            botId = bot.getApi().getMe()->id;
            LogI("Bot username: %s %d", botUsername.c_str(), botId);

            TgLongPoll longPoll(bot);
            while (true)
            {
                LogI("Long poll started.");
                longPoll.start();
            }

            // TgWebhookTcpServer webhookServer(8888, bot);
            // string webhookUrl(getenv("WEBHOOK_URL"));
            // bot.getApi().setWebhook(webhookUrl);
            // webhookServer.start();
        }
        catch (TgException &e)
        {
            LogE("Error: %s", e.what());
        }
        catch (...)
        {
            LogE("Unknow error.");
        }
        LogI("Restart.");
    }

    return 0;
}
