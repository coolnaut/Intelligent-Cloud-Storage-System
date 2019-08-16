//压缩服务总览
#pragma once
#include<iostream>
#include<unordered_map>
#include<string>
#include<sstream>
#include<vector>
#include<pthread.h>
#include<thread>
#include<fstream>
#include<boost/filesystem.hpp>
#include<boost/algorithm/string.hpp>
#include<sys/stat.h>

#include<unistd.h>
#include<sys/file.h>

#define COMPRESS_DIR "mroot/zip/"
#define GUNZIP_DIR "mroot/list"
#define COMMPRESS_FILE_CONF "comconf.txt"
#define HEAT_TIME 60
namespace bf = boost::filesystem;
//压缩服务
class CompressServer
{
	public:
		CompressServer();
		~CompressServer();
		//向外提供获取文件列表的功能
		bool GetFileList(std::vector<std::string>& filelist);
		//向外提供文件下载功能，对文件先进行解压，文件名---文件数据
		bool DowanloadFile(std::string& file, std::string& body);
		//对热度低的文件进行压缩，节省磁盘空间
		bool LowHeatCompress();
		
		bool SetFileData(std::string& file, const std::string& body, int64_t offest);
	private:
		//获取每次存储线程启动时，从文件读取列表信息
		bool GetListConf();
		//压缩完毕，将数据写入文件
		bool SetListConf();
		//文件目录检测
		bool DirCheck();
		//判断文件是否需要被压缩
		bool IsNeedCompress(std::string filename);
		//压缩文件
		bool CompressFile(std::string& filename);
		//解压缩文件
		bool UnCommpressFile(std::string& filename);
		
		bool GetGzpFile(std::string& name, std::string& gzname);
		bool GetNormalFile(std::string& filename, std::string& body);

		bool AddFileConf(std::string& file, const std::string& gzname);
	private:
		std::string file_dir_;
		//保存文件列表，源文件名称，压缩包文件名称---文件映射列表
		std::unordered_map<std::string, std::string> file_list_;
		//文件读写时保护
		pthread_rwlock_t rwlock_;
};
