#pragma once

#include <unordered_map>
#include <string>

// rpcserverip rpcserverport zookeeperip zookeeperport
// 框架读取配置文件
class MprpcConfig
{
public:
    // 加载配置文件
    void LoadConfigFile(const char* config_file);
    // 查询配置文件
    std::string Load(const std::string& key);
private:
    std::unordered_map<std::string, std::string> m_configMap;

};
