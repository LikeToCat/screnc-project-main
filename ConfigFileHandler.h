#pragma once
#include <string>
#include <map>
class ConfigFileHandler
{
public:
    ConfigFileHandler(std::string ConfigPath)
    {
        m_str_ConfigPath = ConfigPath;
    }

    bool ParseData()
    {
        std::ifstream file(m_str_ConfigPath);
        if (!file.is_open())
            return false;

        std::string line;
        std::string currentHeader = "";

        while (std::getline(file, line))
        {

            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            if (line.empty() || line[0] == ';' || line[0] == '#')
                continue;
            if (line[0] == '[' && line[line.size() - 1] == ']')
            {
                currentHeader = line.substr(1, line.size() - 2);
                continue;
            }

            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos)
            {
                std::string key = line.substr(0, equalsPos);
                std::string value = line.substr(equalsPos + 1);
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                m_Map_Config[currentHeader][key] = value;
            }
        }

        file.close();
        return !m_Map_Config.empty();
    }

    bool asBool(std::string header, std::string key)
    {
        auto it = m_Map_Config.find(header);
        if (it != m_Map_Config.end()) 
        {
            auto keyIt = it->second.find(key);
            if (keyIt != it->second.end()) 
            {
                return keyIt->second == "1" ||
                    keyIt->second == "true" ||
                    keyIt->second == "yes" ||
                    keyIt->second == "on";
            }
        }
        return false;
    }
    std::string asString(std::string header, std::string key)
    {
        auto it = m_Map_Config.find(header);
        if (it != m_Map_Config.end()) 
        {
            auto keyIt = it->second.find(key);
            if (keyIt != it->second.end())
            {
                return keyIt->second;
            }
        }
        return "";
    }
    int asInt(std::string header, std::string key)
    {
        auto it = m_Map_Config.find(header);
        if (it != m_Map_Config.end()) 
        {
            auto keyIt = it->second.find(key);
            if (keyIt != it->second.end())
            {
                try
                {
                    return std::stoi(keyIt->second);
                }
                catch (...)
                {
                    return 0;
                }
            }
        }
        return 0;
    }

    bool WriteConfigFile(std::string Header, std::string Key, std::string value)
    {
        if (!ParseData())
            DEBUG_CONSOLE_STR(ConsoleHandle, L"--------警告:解析配置文件失败！可能会出现错误");
        
        // 更新内存中的配置数据
        m_Map_Config[Header][Key] = value;

        // 将更新后的配置写回文件
        std::ofstream writeFile(m_str_ConfigPath);
        if (!writeFile.is_open())
        {
            return false;
        }

        // 遍历所有配置节
        for (const auto& section : m_Map_Config)
        {
            // 写入节名
            writeFile << "[" << section.first << "]" << std::endl;

            // 写入该节下的所有键值对
            for (const auto& entry : section.second)
            {
                writeFile << entry.first << "=" << entry.second << std::endl;
            }

            // 在节之间添加空行，提高可读性
            writeFile << std::endl;
        }

        writeFile.close();
        return true;
    }
    bool DeleteConfigKey(std::string Header, std::string Key)
    {
        // 检查指定的节是否存在
        auto sectionIt = m_Map_Config.find(Header);
        if (sectionIt == m_Map_Config.end())
        {
            return false; // 节不存在
        }

        // 检查指定的键是否存在
        auto keyIt = sectionIt->second.find(Key);
        if (keyIt == sectionIt->second.end())
        {
            return false; // 键不存在
        }

        // 删除指定的键
        sectionIt->second.erase(keyIt);

        // 将更新后的配置写回文件
        std::ofstream writeFile(m_str_ConfigPath);
        if (!writeFile.is_open())
        {
            return false;
        }

        // 遍历所有配置节
        for (const auto& section : m_Map_Config)
        {
            // 跳过空的节
            if (section.second.empty())
            {
                continue;
            }

            // 写入节名
            writeFile << "[" << section.first << "]" << std::endl;

            // 写入该节下的所有键值对
            for (const auto& entry : section.second)
            {
                writeFile << entry.first << "=" << entry.second << std::endl;
            }

            // 在节之间添加空行，提高可读性
            writeFile << std::endl;
        }

        writeFile.close();
        return true;
    }
    bool DeleteConfigHeader(std::string Header)
    {
        // 检查指定的节是否存在
        auto sectionIt = m_Map_Config.find(Header);
        if (sectionIt == m_Map_Config.end())
        {
            return false; // 节不存在
        }

        // 删除整个节
        m_Map_Config.erase(sectionIt);

        // 将更新后的配置写回文件
        std::ofstream writeFile(m_str_ConfigPath);
        if (!writeFile.is_open())
        {
            return false;
        }

        // 遍历所有配置节
        for (const auto& section : m_Map_Config)
        {
            // 写入节名
            writeFile << "[" << section.first << "]" << std::endl;

            // 写入该节下的所有键值对
            for (const auto& entry : section.second)
            {
                writeFile << entry.first << "=" << entry.second << std::endl;
            }

            // 在节之间添加空行，提高可读性
            writeFile << std::endl;
        }

        writeFile.close();
        return true;
    }
private:
    std::map<std::string, std::map<std::string, std::string>> m_Map_Config;
    std::string m_str_ConfigPath;
};