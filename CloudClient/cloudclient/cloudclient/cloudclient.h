#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<unordered_map>
#include<boost/filesystem.hpp>
#include<boost/algorithm/string.hpp>//字符串切割
#include<thread>
#include"httplib.h"

#define BACKUP_PATH "back"		//文件监控与备份目录
#define BACKUP_INFO "backupinfo.txt"		//文件备份信息
#define RANGE_MAX_SIZE (10<<20)
#define SERVER_IP "192.168.138.135"
#define SERVER_PORT 9090
#define BACKUP_URI "/list/"
namespace bf = boost::filesystem;
namespace hb = httplib;
//功能
//获取文件备份信息
//检测目录中文件信息
//判断文件是否需要备份
		//mtime 文件修改时间 与 fsize 文件大小 是否与上次相同	
//备份文件
//记录文件备份信息——etag = mtime + fsize
		//mtime：文件修改时间
		//fsize：文件大小

class CloudClient
{
public:
	CloudClient();
	bool Run();
private:
	//获取备份信息
	bool GetBackupInfo();
	//刷新备份信息
	bool TouchBackupInfo();
	//监听备份目录
	bool BackupDirListen(const std::string& pathname);
	//上传文件/文件备份
	bool PutFileData(const std::string& filename);
		//添加单个文件的etag
		void AddFileBackInfo(const std::string& filename);
		//获取单个文件etag
		std::string GetFileEtag(const std::string& filename);
	//文件是否需要备份
	bool FileDateNeedBackup(const std::string& filename);
private:
	//备份信息
	std::unordered_map<std::string, std::string> backup_list_;
};
