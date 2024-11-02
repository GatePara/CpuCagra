#pragma once

#include <iostream>
#include <string>
#include <typeinfo>
#include "../rapidjson/document.h"
#include "../rapidjson/filereadstream.h"

namespace cpupg
{
    // 定义一个结构体来存储配置信息
    struct CagraConfig
    {
        std::string knng_path;
        std::string knng_format;
        std::string save_path;
        int r_init;
        int r;
    };

    // 从 JSON 文件加载配置
    CagraConfig loadCagraConfig(const char *filename)
    {
        FILE *fp = fopen(filename, "rb"); // 以二进制模式打开文件
        if (!fp)
        {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            exit(1);
        }

        // 创建读取缓冲区
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        // 解析 JSON 文档
        rapidjson::Document document;
        document.ParseStream(is);
        fclose(fp);

        // 检查 "cagra" 对象是否存在
        if (!document.HasMember("cagra") || !document["cagra"].IsObject())
        {
            std::cerr << "Error: Invalid JSON format, missing 'cagra' object." << std::endl;
            exit(1);
        }

        // 提取 "cagra" 对象中的配置项
        CagraConfig config;
        const rapidjson::Value &cagra = document["cagra"];

        // 读取 KNNG_PATH
        if (cagra.HasMember("KNNG_PATH") && cagra["KNNG_PATH"].IsString())
        {
            config.knng_path = cagra["KNNG_PATH"].GetString();
        }
        else
        {
            std::cerr << "Error: KNNG_PATH not found or not a string." << std::endl;
        }

        // 读取 KNNG_FORMAT
        if (cagra.HasMember("KNNG_FORMAT") && cagra["KNNG_FORMAT"].IsString())
        {
            config.knng_format = cagra["KNNG_FORMAT"].GetString();
        }
        else
        {
            std::cerr << "Error: KNNG_FORMAT not found or not a string." << std::endl;
        }

        // 读取 SAVE_PATH
        if (cagra.HasMember("SAVE_PATH") && cagra["SAVE_PATH"].IsString())
        {
            config.save_path = cagra["SAVE_PATH"].GetString();
        }
        else
        {
            std::cerr << "Error: SAVE_PATH not found or not a string." << std::endl;
        }

        // 读取 R_INIT
        if (cagra.HasMember("R_INIT") && cagra["R_INIT"].IsInt())
        {
            config.r_init = cagra["R_INIT"].GetInt();
        }
        else
        {
            std::cerr << "Error: R_INIT not found or not an integer." << std::endl;
        }

        // 读取 R
        if (cagra.HasMember("R") && cagra["R"].IsInt())
        {
            config.r = cagra["R"].GetInt();
        }
        else
        {
            std::cerr << "Error: R not found or not an integer." << std::endl;
        }

        return config;
    }
} // namespace cpupg
