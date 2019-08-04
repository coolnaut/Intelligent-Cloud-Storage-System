#include"cloudclient.h"




bool CloudClient::Run()
{
	GetBackupInfo();
	while (1)
	{
		BackupDirListen(BACKUP_PATH);
		TouchBackupInfo();
		Sleep(10000);
	}
	return true;
}


class ThreadPut
{
private:
	std::string file_;
	int64_t range_start_;
	int64_t range_len_;
public:
	bool res_;
public:
	ThreadPut()
	{}
	ThreadPut(const std::string& filename, int64_t range_start, int64_t range_len)
		:file_(filename)
		,range_start_(range_start)
		, range_len_(range_len)
		,res_(true)
	{}
	bool Start()
	{
		std::ifstream file(file_, std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "线程打开文件失败" << std::endl;
			res_ = false;
			return res_;
		}
		file.seekg(range_start_, std::ios::beg);

		std::string body;
		body.resize(range_len_);
		file.read(&body[0], range_len_);
		if(!file.good())
		 {
			std::cout << "线程读取文件失败" << std::endl;
			res_ = false;
			return res_;
		}
		std::cout << "线程读取文件完毕" << std::endl;
		file.close();
		bf::path path(file_);
		std::string url = BACKUP_URI + path.filename().string();

		//实例化客户端
		hb::Client client(SERVER_IP, SERVER_PORT);
		//顶http头信息
		hb::Headers hdr;
		hdr.insert(std::make_pair("Content-Length", std::to_string(range_len_)));

		std::stringstream tmp;
		tmp << "bytes=" << range_start_ << "-" << range_start_ + range_len_ - 1;
		hdr.insert(std::make_pair("Range", tmp.str().c_str()));
		auto rsp = client.Put(url.c_str(), hdr, body, "text/plain");
		std::cout << file_<<"=====" << range_start_ << "----" << range_len_ << "正在上传" << std::endl;
		if (rsp && rsp->status == 200)
			res_ = true;
		else
			res_ = false;
		return true;
	}

};

CloudClient::CloudClient()
{
	bf::path path(BACKUP_PATH);
	if (!bf::exists(path))
		bf::create_directory(path);
}
bool CloudClient::GetBackupInfo()
{
	bf::path path_(BACKUP_INFO);
	if (!bf::exists(path_))
	{
		std::cout << "备份配置信息文件不存在" << std::endl;
		return false;
	}
	int64_t fsize = bf::file_size(path_);
	if (fsize == 0)
	{
		std::cout << "备份文件中没有信息条目" << std::endl;
		return false;
	}
	std::string body;
	body.resize(fsize);
	std::ifstream Infile(BACKUP_INFO, std::ios::binary);
	if (!Infile.is_open())
	{
		std::cout << "备份文件打开失败" << std::endl;
		return false;
	}
	Infile.read(&body[0], fsize);
	if (!Infile.good())
	{
		std::cout << "备份信息条目读取失败" << std::endl;
		return false;
	}

	std::vector<std::string> list;
	boost::split(list, body, boost::is_any_of("\n"));

	for (const auto& str : list)
	{
		size_t pos = str.find(" ");
		if (pos == std::string::npos)
			continue;
		std::string key = str.substr(0, pos);
		std::string val = str.substr(pos+1);
		std::cout << key << "--->" << val << std::endl;
		backup_list_[key] = val;
	}
	std::cout << "备份信息条目读取成功" << std::endl;
	return true;
}
bool CloudClient::TouchBackupInfo()
{
	std::cout << "正在刷新备份信息文件#################" << std::endl;
	std::string body;
	for (const auto& e : backup_list_)
	{
		body += e.first + " " + e.second + "\n";
	}
	std::ofstream Outfile(BACKUP_INFO, std::ios::binary);
	if (!Outfile.is_open())
	{
		std::cout << "备份文件打开失败" << std::endl;
		return false;
	}
	Outfile.write(&body[0], body.size());
	if(!Outfile.good())
	{
		std::cout << "备份文件写入失败" << std::endl;
		return false;
	}
	std::cout << "备份文件成功刷新###################" << std::endl;
	return true;
}
bool CloudClient::BackupDirListen(const std::string& pathname)
{
	bf::directory_iterator dit_begin(pathname);
	bf::directory_iterator dit_end;
	//监听目录是否有修改过的或者新文件
	for (;dit_begin != dit_end; ++dit_begin)
	{
		//如果是目录则递归进行监听
		if (bf::is_directory(dit_begin->status()))
		{
			BackupDirListen(dit_begin->path().string());			
			continue;
		}
		//判断是否需要备份
		if (FileDateNeedBackup(dit_begin->path().string()))
		{
			if (false == PutFileData(dit_begin->path().string()))
			{
				std::cout << dit_begin->path().string() << "备份失败！！" << std::endl;
				continue;
			}
			std::cout << dit_begin->path().string() << "备份成功！！" << std::endl;
			AddFileBackInfo(dit_begin->path().string());
		}
	}
	return true;
}
void CloudClient::AddFileBackInfo(const std::string& filename)
{
	std::cout << filename << "======" << "正在添加etag" << std::endl;
	std::string etag = GetFileEtag(filename);
	backup_list_[filename] = etag;
	return;
}
std::string  CloudClient::GetFileEtag(const std::string& filename)
{
	bf::path path(filename);
	//文件大小
	int64_t fsize = bf::file_size(path);
	//最后写入时间
	int64_t ltime = bf::last_write_time(path);

	std::stringstream etag;
	etag << std::hex << fsize << "-" << std::hex << ltime;
	//返回etag信息
	return etag.str();
}
//现在入口函数
static void thread_main(ThreadPut* pthreadup)
{
	pthreadup->Start();
	return;
}
bool CloudClient::PutFileData(const std::string& filename)
{
	//按分块传输大小对文件内容进行分块传输
	//选择多线程处理
	//1.获取文件大小
	std::cout << "正在准备文件上传##############" << std::endl;
	int64_t fsize = bf::file_size(filename);
	if (fsize <= 0)
	{
		std::cout << filename  <<"不存在" << std::endl;
		return false;
	}
	//2.计算分块个数，得到起始位置，与块大小
	int count = (int)fsize / RANGE_MAX_SIZE;

	std::vector< ThreadPut> thread_put;
	std::vector< std::thread> thread_list;
	for (int i = 0; i <= count; ++i)
	{
		int64_t range_begin = i * RANGE_MAX_SIZE;
		int64_t range_len = (i + 1) * RANGE_MAX_SIZE - 1;
		if (i == count)
		{
			range_len = fsize - 1;
		}
		int64_t len = range_len - range_begin + 1;
		ThreadPut back_info(filename, range_begin, len);
		thread_put.push_back(back_info);
	}
	for (int i = 0; i <= count; ++i)
	{
		thread_list.push_back(std::thread(thread_main, &thread_put[i]));
	}
	//3.等待线程退出，判断文件上传结果
	bool res = true;;
	for (int i = 0; i <= count; ++i)
	{
		thread_list[i].join();
		if (thread_put[i].res_ == true)
			continue;
		res = false;
	}
	//4.上传成功，添加文件的备份信息
	return res;
}
bool CloudClient::FileDateNeedBackup(const std::string& filename)
{
	//获取当前文件的etag信息
	std::string etag = GetFileEtag(filename);
	//在程序中的backup_list_是否有当前文件对象
	auto tmp = backup_list_.find(filename);
	//有且和backup_list_中的etag信息相同，则不需要备份
	if (tmp != backup_list_.end() && tmp->second == etag)
		return false;
	return true;
}