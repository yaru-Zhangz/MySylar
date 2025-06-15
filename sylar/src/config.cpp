#include "config.h"

namespace sylar {

// 通用查找，返回基类指针
ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    std::shared_lock<std::shared_mutex> lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

/*
    递归遍历yaml节点树，将所有叶节点收集到一个列表中，并构建完整的点分割路径名
    示例：
        输入YAML:
            server:
                port: 8080
                log_level: info
        输出列表：
            [
                {"server", MapNode},
                {"server.port", ScalarNode(8080)},
                {"server.log_level", ScalarNode("info")}
            ]
*/
static void ListAllMember(const std::string& prefix, const YAML::Node& node, std::list<std::pair<std::string, const YAML::Node>>& output) {

    if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return; 
    }

    output.push_back(std::make_pair(prefix, node));
    if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); it++) {
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}

// 从yaml根节点加载配置，更新已注册的配置变量
void Config::LoadFromYaml(const YAML::Node& root) {
    // 递归收集所有节点到列表
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    // 遍历所有节点，更新配置
    for(auto& i : all_nodes) {
        std::string key = i.first;
        if(key.empty()) continue;   // 跳过根节点

        std::transform(key.begin(), key.end(), key.begin(), ::tolower); // 统一转化为小写

        // 从全局配置表中查找对应名称的变量
        ConfigVarBase::ptr var = Config::LookupBase(key);

        if(var) {
            // 节点类型为标量
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {   // 节点类型非标量，序列化为字符串再解析配置值
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    std::shared_lock<std::shared_mutex> lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin(); it != m.end(); it++) {
        cb(it->second);
    }  
}

}