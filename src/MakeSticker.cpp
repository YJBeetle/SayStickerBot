#include "MakeSticker.h"

#include <iostream>
#include <algorithm>

#include "Log.h"
#include "Global.h"
#include "Tg.h"
#include "StringCheck.h"

using namespace std;
using namespace cv;
using namespace TgBot;

// ArtRobot的实际绘制函数
std::tuple<shared_ptr<ArtRobot::Component::Base>, double, double>
drawImage(const string &__userPhotoData, const string &__name, const string &__content)
{
    // 首先创建文字，因为需要计算其中宽高
    auto textName = make_shared<ArtRobot::Component::Text>("textName",
                                                           125, 59, 0, // 坐标
                                                           __name,
                                                           "BD4332",
                                                           "Noto Sans CJK SC",
                                                           600,  // 字重
                                                           28,   // 字号
                                                           0, 0, // 对齐方式
                                                           346); // 最大宽度346
    auto content = make_shared<ArtRobot::Component::TextArea>("content",
                                                              125, 71, 346, 417, 0, // 坐标
                                                              __content,
                                                              "000000",
                                                              "Noto Sans CJK SC",
                                                              400,  // 字重
                                                              28,   // 字号
                                                              0, 0, // 对齐方式
                                                              -5);  // 行距-5

    double ws = 346 - max(content->realW(), textName->realW());
    if (ws < 0)
        ws = 0;
    double hs = 407 - content->realH();
    if (hs < 0)
        hs = 0;

    cout << ws << " " << hs << endl;

    auto body = make_shared<ArtRobot::Component::Group>("body"); // body

    auto bg = make_shared<ArtRobot::Component::Rectangle>("bg", 0, 0, 512 - ws, 512 - hs, 0, "79B3E2"); // bg
    body->addChild(bg);

    vector<unsigned char> userPhotoData(__userPhotoData.begin(), __userPhotoData.end()); // 图片转为vector
    Mat userPhotoMat = imdecode(userPhotoData, IMREAD_COLOR);                            // 图片转为Mat
    auto userPhoto = make_shared<ArtRobot::Component::Image>("userPhoto",                // userPhoto
                                                             16, 430 - hs,
                                                             66, 66,
                                                             0,
                                                             userPhotoMat);
    auto userPhotoMask = make_shared<ArtRobot::Component::ImageMask>("userPhotoMask", // userPhotoMask
                                                                     16, 430 - hs,
                                                                     66, 66,
                                                                     0,
                                                                     "SaySticker2_userPhotoMask.png",
                                                                     userPhoto);
    body->addChild(userPhotoMask);

    auto messageShadow = make_shared<ArtRobot::Component::Rectangle>("messageShadow", 88, 496 - hs, 398 - ws, 4, 0, "00000010"); // messageShadow
    body->addChild(messageShadow);

    auto messageShadow2 = make_shared<ArtRobot::Component::RectangleRound>("messageShadow2", 486 - ws, 486 - hs, 10, 14, 0, 0, 0, 10, 0, "00000010"); // messageShadow
    body->addChild(messageShadow2);

    auto messageX = make_shared<ArtRobot::Component::Image>("messageX", // messageX
                                                            88, 474 - hs,
                                                            12, 22,
                                                            0,
                                                            "SaySticker2_messageX.png");
    body->addChild(messageX);

    auto messageBG = make_shared<ArtRobot::Component::RectangleRound>("messageBG", 100, 16, 396 - ws, 480 - hs, 0, 10, 10, 10, 0, "FFFFFF"); // messageBG
    body->addChild(messageBG);

    body->addChild(textName);
    body->addChild(content);

    double realWidth = 512 - ws,
           realHeight = 512 - hs;

    return {body, realWidth, realHeight};
}

string MakeSticker(const Api &api, int64_t chatId,
                   User::Ptr user,
                   const string &content,
                   int32_t ownerId)
{
    LogV("%s %d", user->username.c_str(), user->id);

    if (!checkSelf(user->id))
    {
        sendMessage(api, chatId, "哼～ (┙>∧<)┙彡 ┻━┻"); // 不允许收录自己
        return "";
    }

    sendChatActionUploadPhoto(api, chatId); // 设置正在发送

    UserProfilePhotos::Ptr userPhotosInfo; // 获取用户头像信息
    try
    {
        userPhotosInfo = api.getUserProfilePhotos(user->id);
    }
    catch (TgException &e)
    {
        LogE("TgBot::Api::getUserProfilePhotos: %s", e.what());
        return "";
    }

    string userPhotoData;           // 用户头像数据
    if (userPhotosInfo->totalCount) // 照片数不为0
    {
        try
        {
            auto &userPhotosInfoFirst = userPhotosInfo->photos[0];
            auto userPhotosPath = api.getFile(userPhotosInfoFirst[userPhotosInfoFirst.size() - 1]->fileId); // 取用最大的图片
            userPhotoData = api.downloadFile(userPhotosPath->filePath);                                     // 图像数据（maybe jpg）
        }
        catch (TgException &e)
        {
            LogE("Get user photo error: %s", e.what());
            return "";
        }
    }
    else
    {
        LogW("No photos.");
        sendMessage(api, chatId, "他没有头像 (◞‸◟)");
        return "";
    }

    string title = user->username.empty() ? "Say" : "Say @" + user->username;
    string username = getUsername(user); // username

    sendChatActionUploadPhoto(api, chatId); // 设置正在发送

    string showName; // 显示名字
    if (user->firstName.length() && user->lastName.length())
        showName = user->firstName + " " + user->lastName;
    else if (user->firstName.length())
        showName = user->firstName;
    else if (user->lastName.length())
        showName = user->lastName;
    else
        showName = username;

    lowercase(username);
    string stickerName = getStickerName(username); // 贴纸名字

    LogV("title=%s, ownerId=%d, stickerName=%s", title.c_str(), ownerId, stickerName.c_str());

    auto [body, realWidth, realHeight] = drawImage(userPhotoData, showName, content); // 绘制图像

    ArtRobot::Renderer renderer(ArtRobot::OutputTypePng, 512, realHeight, ArtRobot::Renderer::PX, 72); // 渲染png
    renderer.render(body->getSurface());

    auto stickerPngFile = make_shared<InputFile>(); // 待上传的PNG文件
    stickerPngFile->data = renderer.getDataString();
    stickerPngFile->mimeType = "image/png";

    File::Ptr stickerFile;
    try
    {
        stickerFile = api.uploadStickerFile(ownerId, stickerPngFile); // 上传贴纸
    }
    catch (TgException &e)
    {
        LogE("TgBot::Api::uploadStickerFile: %s", e.what());
        return "";
    }

    StickerSet::Ptr stickerSet;
    try
    {
        stickerSet = api.getStickerSet(stickerName); // 尝试获取贴纸包
    }
    catch (TgException &e)
    {
        LogI("TgBot::Api::getStickerSet: %s", e.what());
        LogI("No sticker, create it.");
    }

    if (stickerSet)
    { // 存在贴纸包
        try
        {
            api.addStickerToSet(ownerId, stickerName, stickerFile->fileId, "💬"); // 添加贴纸到贴纸包
        }
        catch (TgException &e)
        {
            LogE("TgBot::Api::addStickerToSet: %s", e.what());
            return "";
        }
        // for (auto sticker : stickerSet->stickers)
        //     try
        //     {
        //         api.deleteStickerFromSet(sticker->fileId); // 删除所有其他贴纸
        //     }
        //     catch (TgException &e)
        //     {
        //         LogE("TgBot::Api::deleteStickerFromSet: %s", e.what());
        //     }
    }
    else
    { // 没有找到贴纸 创建
        try
        {
            api.createNewStickerSet(ownerId, stickerName, title, stickerFile->fileId, "💬"); // 创建贴纸包并添加第一个贴纸
        }
        catch (TgException &e)
        {
            LogE("TgBot::Api::createNewStickerSet: %s", e.what());
            if (strcmp(e.what(), "Bad Request: PEER_ID_INVALID") == 0)
                sendMessage(api, chatId, "贴图创建失败，原因是Telegram的服务器拒绝将你添加为该表情的所有者");
            return "";
        }
    }

    string stickerFileId;
    try
    {
        auto sickers = api.getStickerSet(stickerName)->stickers;
        stickerFileId = sickers[sickers.size() - 1]->fileId;
    }
    catch (TgException &e)
    {
        LogE("TgBot::Api::getStickerSet: %s", e.what());
        return "";
    }

    return stickerFileId;
}
