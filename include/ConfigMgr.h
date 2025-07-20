#pragma once
#include "const.h"
// 存储section和key-value对的map
// 造轮子
struct SectionInfo {
    SectionInfo(){}
    ~SectionInfo(){
        _section_datas.clear();
    }
    SectionInfo(const SectionInfo& src) {
        _section_datas = src._section_datas;
    }
    SectionInfo& operator = (const SectionInfo& src) {
        if (&src == this) {
            return *this;
        }
        this->_section_datas = src._section_datas;
        return *this;
    }
    std::map<std::string, std::string> _section_datas;
    std::string  operator[](const std::string  &key) {
        if (_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        // 这里可以添加一些边界检查  
        return _section_datas[key];
    }
};

class ConfigMgr
{
public:
    ~ConfigMgr() {
        _config_map.clear();
    }
    SectionInfo operator[](const std::string& section) {
        if (_config_map.find(section) == _config_map.end()) {
            return SectionInfo();
        }
        return _config_map[section];
    }
    //通过函数实现单例的设计与完成
    static ConfigMgr& Inst() {
        static ConfigMgr cfg_mgr; // 静态变量，只会初始化一次，且在程序结束时销毁
        return cfg_mgr;
    }
private:
    // 存储section和key-value对的map  
    std::map<std::string, SectionInfo> _config_map;
    ConfigMgr& operator=(const ConfigMgr& src) {
        if (&src == this) {
            return *this;
        }
        this->_config_map = src._config_map;
    };//赋值运算符重载
    ConfigMgr(const ConfigMgr& src) {
        this->_config_map = src._config_map;
    }//拷贝构造函数
    ConfigMgr();//构造函数
};