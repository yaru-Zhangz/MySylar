#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include<memory>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<yaml-cpp/yaml.h>
#include "log.h"
namespace sylar {

// 作为所有配置变量的基类，定义基本接口
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
private:
    std::string m_name;
    std::string m_description;
};


template<class T>
class ConfigVar : public ConfigVarBase
{
public:
    using ptr = std::shared_ptr<ConfigVar>;
    ConfigVar(const std::string& name
            , const T& default_value
            , const std::string& description = "")
            :ConfigVarBase(name, description)
            ,m_val(default_value) {
            }
    
    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << "convert: " << typeid(m_val).name() << " to string";
            }
            return "";

    }

    bool fromString(const std::string& val) override {
        try {
            m_val = boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << "convert: string to" << typeid(m_val).name();
            }
            return false;
    }

    const T getValue() const { return m_val;}
    void setValue(const T& v) { m_val = v; }
private:
    T m_val;
};

//  
class Config {
public:
    using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

    // 查找配置项（带默认值）
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "") {
            auto tmp = Lookup<T>(name);
            if(tmp) {   // 存在同名配置项，返回现有配置项
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists";
                return tmp;
            }
            // 验证名称的合法性
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }
            // 嵌套从属类型名称 前面用typename
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            s_datas[name] = v;
            return v; 
            }
    
    // 查找配置项（不带默认值）
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if(it == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap s_datas;
};


} // end sylar

#endif