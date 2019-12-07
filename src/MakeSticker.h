#pragma once

#include <tgbot/tgbot.h>
#include <opencv2/opencv.hpp>
#include <ArtRobot/ArtRobot.h>

bool MakeSticker(const TgBot::Api &api, int64_t chatId,
                 TgBot::User::Ptr user,
                 const std::string &content,
                 int32_t ownerId);
