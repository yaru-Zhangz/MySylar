#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<yaml-cpp/yaml.h>
#include<vector>
#include<map>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<list>
#include<functional>

#include "log.h"
#include "thread.h"

namespace sylar {

/*
    "约定优于配置"的思路体现为所有的配置项在定义时都带一个的默认值
*/

// 配置项基类
class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name;}
    const std::string& getDescription() const {return m_description;}

    virtual std::string toString() = 0; // 将配置值转化为字符串
    virtual bool fromString(const std::string& val) = 0; // 从字符串解析配置值
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;
    std::string m_description;
};

// 类型转换模板类(F 源类型, T 目标类型)
// 实现基本数据类型和std::string之间的相互转换
template<class F, class T>
class LexicalCast {
public:
    T operator() (const F& v) {
        return boost::lexical_cast<T> (v);
    }
};

// 通过偏特化实现std::string和STL容器之间的转化
// vector
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator() (const std::vector<T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// list
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator() (const std::list<T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// set
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator() (const std::set<T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_set
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); i++) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator() (const std::unordered_set<T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// map
template<class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
std::map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); it++) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator() (const std::map<std::string, T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_map
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
std::unordered_map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); it++) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator() (const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 模版化配置项
template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
    using ptr = std::shared_ptr<ConfigVar>;
    using  on_change_cb = std::function<void (const T& old_value, const T& new_value)>;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
            :ConfigVarBase(name, description), m_val(default_value) {
    }
    
    std::string toString() override {
        try {
            std::shared_lock<std::shared_mutex> lock(rw_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << "convert: " << typeid(m_val).name() << " to string";
            }
            return "";

    }

    bool fromString(const std::string& val) override {
        try {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << "convert: string to" << typeid(m_val).name();
            }
            return false;
    }

    const T getValue() { 
        std::shared_lock<std::shared_mutex> lck(rw_mutex);
        return m_val;
    }
    
    void setValue(const T& v) { 
        {
            std::shared_lock<std::shared_mutex> lck(rw_mutex);
            if(v == m_val) {    // 值没有变化直接返回
                return;
            }
            // 遍历回调映射表，执行回调
            for(auto& i : m_cbs) {  
                i.second(m_val, v);
            }
        }
        std::unique_lock<std::shared_mutex> lck(rw_mutex);
        m_val = v;
    }
    std::string getTypeName() const override {return typeid(T).name();}

    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        std::unique_lock<std::shared_mutex> lck(rw_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    void delListener(uint64_t key) {
        std::unique_lock<std::shared_mutex> lck(rw_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) 
    {
        std::shared_lock<std::shared_mutex> lck(rw_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        std::unique_lock<std::shared_mutex> lck(rw_mutex);
        m_cbs.clear();
    }
private:
    T m_val;
    std::shared_mutex rw_mutex;
    // 变更回调函数组
    std::map<uint64_t, on_change_cb> m_cbs;
};

// 配置管理器
class Config {
public:
    // 通过多态统一管理，进行类型擦除
    using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

    // 带默认值的查找
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "") {
            std::unique_lock<std::shared_mutex> lock(GetMutex());
            auto it = GetDatas().find(name);
            if(it != GetDatas().end()) { // 
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second); // ​​将基类指针ConfigVarBase::ptr安全地向下转型为派生类指针 ConfigVar<T>::ptr​​, dynamic_cast 仅支持裸指针
                if(tmp) {   // 如果存在且类型匹配，返回现有配置项。
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists";
                    return tmp;
                } else {    // 如果存在但类型不匹配，返回 nullptr 并记录错误日志。
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists but type not " 
                                << typeid(T).name() << " real_type=" << it->second->getTypeName()
                                << " " << it->second->toString();
                    return nullptr;
                }
            }
            
            // 验证名称的合法性
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            // 创建新配置项并注册到全局 ConfigVarMap
            // 嵌套从属类型名称, 依赖模板参数T的名称， 前面用typename, 用于告诉编译器 ​​ConfigVar<T>::ptr 是一个类型名​​，而不是静态成员变量或其他名称。
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = v;
            return v; 
            }
    
    // 不带默认值的查找
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        std::shared_lock<std::shared_mutex> lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
   // 使用静态局部变量 GetDatas() 和 GetMutex() 保证线程安全的延迟初始化
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
    static std::shared_mutex& GetMutex() {
        static std::shared_mutex s_mutex;
        return s_mutex;
    }
};


} // end sylar

#endif