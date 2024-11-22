#include "mprpcconfig.h"
#include <iostream>
// 加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (pf == nullptr)
    {
        exit(EXIT_FAILURE);
    }

    // 1.注释 2.正确配置项 3. 去掉开头空格
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);
        std::string src_buf(buf);
        // 去掉开头空格
        int idx = src_buf.find_first_not_of(' ');
        if (idx != -1)
        {
            src_buf = src_buf.substr(idx,src_buf.size() - idx);
        }
        // 去掉结尾空格
        idx = src_buf.find_last_not_of(' ');
        if (idx != -1)
        {
            src_buf = src_buf.substr(0, idx + 1);
        }
        // 去掉中间空格
        idx = src_buf.find_first_of(' ');
        while (idx != -1)
        {
            src_buf =src_buf.erase(idx, 1);
            idx = src_buf.find_first_of(' ');
        }
        // 判断是否是注释或者全是空格
        if (src_buf[0] == '#' || src_buf.empty())
        {
            continue;
        }
        // 去掉换行
        idx = src_buf.find('\n');
        if (idx != -1)
        {
            src_buf = src_buf.erase(idx);
        }
        // 解析配置项
        idx = src_buf.find('=');
        if (idx == -1)
        {
            std::cout << "invalid!" << std::endl;
            continue;
        }
        std::string key = src_buf.substr(0, idx);
        std::string value = src_buf.substr(idx + 1, src_buf.size() - idx);
        m_configMap.insert({key, value});
        
        
    }
    

}
// 查询配置文件
std::string MprpcConfig::Load(const std::string& key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) 
    {
        return "";
    }
    return it->second; 
}