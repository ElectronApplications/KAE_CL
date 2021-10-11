#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <unistd.h>
#include <curl/curl.h>
#include <json/json.h>

#include "main.h"
#include "clminer.h"

const std::string URL = "https://kae.nk.ax";

Block block;

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cout << "Usage: " << argv[0] << " address" << std::endl;
        return 0;
    }
    std::string address = argv[1];

    initCL();

    std::thread updater(updateBlock);
    while(true) {
        block.fromJson(request(URL + "/currentblock"));
        std::cout << "New block [" << block.id << "]. Difficulty: " << block.difficulty << ", reward: " << std::to_string((double) block.reward / 1000) << " KAE" << std::endl;
        std::string pass = runCL();
        if(pass != "") {
            request(URL + "/submitblock/" + pass + "/" + address);
            std::cout << "FOUND " << pass << std::endl;
        }
    }
    
    return 0;
}

void updateBlock() {
    Block newBlock;
    while(true) {
        newBlock.fromJson(request(URL + "/currentblock"));
        if(newBlock.id != block.id) {
            std::cout << "New Block!" << std::endl;
            block = newBlock;
            stopCL();
        }
        sleep(2);
    }
}

void Block::fromJson(std::string jsonStr) {
    Json::Value json;
    Json::CharReaderBuilder builder;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
    reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &json, nullptr);

    this->createdTimestamp = json["createdTimestamp"].asInt64();    
    this->difficulty = json["difficulty"].asInt64();
    this->hash = json["hash"].asString();
    this->id = json["id"].asInt64();
    this->reward = json["reward"].asInt64();
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string request(std::string url) {
    CURL *curl;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        return readBuffer;
    }
    return "";
}
