#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <json/json_features.h>

struct Block {
    long id;
    long difficulty;
    long createdTimestamp;
    long reward;
    std::string hash;

    void fromJson(std::string jsonStr);
};

extern Block block;

void updateBlock();
std::string request(std::string url);

#endif