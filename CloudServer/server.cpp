

#include "server.h"
#include "compressserver.h"


CompressServer csr;
//class Server
Server::Server()
{
	bf::path rootpath_(SERVER_ROOT_PATH);
	bf::path backuppath_(SERVER_BACKUP_PATH);
	if(!bf::exists(rootpath_))
		bf::create_directory(rootpath_);
	if(!bf::exists(backuppath_))
		bf::create_directory(backuppath_);
	
}
bool Server::Run()
{
	//设置根目录
	srv_.set_base_dir(SERVER_ROOT_PATH);
	//正则表达式，请求/或者/list均展示文件列表
	srv_.Get("/(list(/){0,1}){0,1}",GetFileList);
	//下载文件
	srv_.Get("/list/(.*)",GetFileData);
	srv_.Get("/data/(.*)",GetFileData);
	//上传文件
	srv_.Put("/list/(.*)",PutFileData);
	//开始监听
	srv_.listen(SERVER_ADDR, SERVER_PORT);
	return true;
}
void Server::GetFileList(const hb::Request& req, hb::Response& res)
{
	std::string body;
	std::vector<std::string> list;
	csr.GetFileList(list);
	body = "<html><head><meta http-equiv='content-type' content='text/html;charset=utf-8'>";
	body += "<style type='text/css'>#div1{position: relative;width: 1530px;height: 750px;}";
	body += "#img1{width: 100%;height: 100%;}#span3{position: absolute;width: 100%;bottom: 250px;left: 0px;text-align: center;font-size: 15px;color:white;}";
	body += "#copy-right-grids {position:absolute;left:40%;bottom:10000;text-align :center;} </style></head><body>";
	body += "<div><div id='div1'> <img src=\"./data/cloud.png\"  id='img1' /></div><div id = 'span3'><h1><font color=pink>文件列表，点击即可下载</font></h1>";
	
	for(const auto& f : list)
	{
		bf::path path(f);
		std::string filename = path.filename().string();
		std::string uri = "/list/" + filename;
		body += "<h4><li>";
		body += "<a href='";
		body += uri;
		body += "'>";
		body += filename;
		body += "</a></li></h4>";

	}
	body += "</div></body></html>";
	res.set_content(&body[0],"text/html");
}
void Server::GetFileData(const hb::Request& req, hb::Response& res)
{
	std::string file = SERVER_ROOT_PATH + req.path;
	//std::string body;
	csr.DowanloadFile(file, res.body);
	std::cout << file << " res.body.size=============" << res.body.size() << std::endl;
	
	res.set_header("Content-Length", std::to_string(res.body.size()).c_str());
	res.set_header("Content-Type", "application/octet-stream");
}
void Server::PutFileData(const hb::Request& req, hb::Response& res)
{
	int64_t range_bigin;
	if(req.has_header("Range"))
	{
		std::string range = req.get_header_value("Range");
		if(!RangeParse(range,range_bigin))
		{
			res.status = 400;	
			return;
		}
	}
	else
	{
		res.status = 400;	
		return;
	}
	std::string realpath = SERVER_ROOT_PATH + req.path;	
	
	csr.SetFileData(realpath, req.body,range_bigin);
	return ;
}
bool  Server::RangeParse(std::string& range, int64_t& begin)
{
	//range: unit = <range-start>-<range-end>
	size_t pos1 = range.find("=");
	size_t pos2 = range.find("-");
	if(pos1 == std::string::npos || pos2 == std::string::npos)
	{
		std::cout << "find error" << std::endl;
		return false;
	}
	std::stringstream start;
	start << range.substr(pos1 + 1, pos2 - pos1 - 1);
	start >> begin;
	return true;
}

void tht_start()
{
	csr.LowHeatCompress();
}
int main()
{
	std::thread thr(tht_start);
	thr.detach();
	Server srv;
	srv.Run();
	return 0;
}
