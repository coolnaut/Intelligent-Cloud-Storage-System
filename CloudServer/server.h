/*░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
▩ File Name: server.h
▩ Author: top-down
▩ Description: 
▩ Created Time: 2019年07月15日 星期一 20时40分11秒
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░*/

#include<iostream>
#include<boost/filesystem.hpp>
#include<sstream>
#include<unistd.h>
#include<fcntl.h>
#include<fstream>
#include "httplib.h"
#include "compressserver.h"

#define SERVER_ROOT_PATH "mroot"
#define SERVER_BACKUP_PATH "mroot/list"
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 9090

namespace hb = httplib;
namespace bf = boost::filesystem;


class Server
{
	public:
		Server();
		bool Run();
	private:
		//展示文件列表
		static void GetFileList(const hb::Request& req, hb::Response& res);
		//获取文件数据
		static void GetFileData(const hb::Request& req, hb::Response& res);
		//文件上传
		static void PutFileData(const hb::Request& req, hb::Response& res);
		static bool  RangeParse(std::string& range, int64_t& begin);
		//文件备份
		static void BackupFile(const hb::Request& req, hb::Response& res);
	private:
		hb::Server srv_;	
};
