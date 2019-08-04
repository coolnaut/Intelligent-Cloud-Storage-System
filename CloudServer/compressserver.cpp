
#include"compressserver.h"

////////////////////////////////////////////////////////////
//共有成员函数
///////////////////////////////////////////////////////////
CompressServer::CompressServer()
{
	pthread_rwlock_init(&rwlock_, NULL);
	if(bf::exists(COMPRESS_DIR))
	{
		bf::create_directory(COMPRESS_DIR);
	}
}
CompressServer::~CompressServer()
{
	pthread_rwlock_destroy(&rwlock_);
}

bool CompressServer::GetFileList(std::vector<std::string>& filelist)
{
	pthread_rwlock_rdlock(&rwlock_);
	for(const auto file : file_list_)
	{
		filelist.push_back(file.first);
	}
	pthread_rwlock_unlock(&rwlock_);
}
bool CompressServer::DowanloadFile(std::string& file, std::string& body)
{
	if(bf::exists(file.c_str()))
	{
		//非压缩文件获取
		GetNormalFile(file, body);
	}
	else
	{	//压缩文件获取
		std::string gzfile;
		GetGzpFile(file,gzfile);
		UnCommpressFile(gzfile);
		std::cout << file  << " will Dowanload"<< std::endl;
		GetNormalFile(file, body);
	}
	return true;
}

bool CompressServer::LowHeatCompress()
{
	//获取list目录下文件
	GetListConf();
	while(1)
	{
		//目录检测，对文件进行压缩
		sleep(5);
		DirCheck();
		SetListConf();
	}
}
 bool CompressServer::SetFileData(std::string& file, const std::string& body, int64_t offest)
 {
	int fd = open(file.c_str(), O_CREAT | O_WRONLY, 0664);
	if(fd < 0)
	{
		std::cout << "SetFileData open error" << std::endl;
		return false;
	}
	lseek(fd, offest, SEEK_SET);
	int ret = write(fd, &body[0], body.size());
	if(ret < 0)
	{
		std::cout << "SetFileData write error" << std::endl;
		return false;
	}
	close(fd);
	return true;
 }



////////////////////////////////////////////////////////////
//私有成员函数
///////////////////////////////////////////////////////////
bool CompressServer::GetListConf()
{
	bf::path file(COMMPRESS_FILE_CONF);
	if(!bf::exists(file))
	{
		std::cout << file << " not exists" << std::endl;
		return false;
	}
	std::ifstream infile(COMMPRESS_FILE_CONF, std::ios::binary);
	if(!infile.is_open())
	{
		std::cout << "open COMMPRESS_FILE_CONF error" << std::endl;
		return false;
	}
	int64_t fsize = bf::file_size(file);
	std::string body;
	body.resize(fsize);
	infile.read(&body[0], fsize);
	if(!infile.good())
	{
		std::cout << "COMMPRESS_FILE_CONF read error" << std::endl;
		return false;
	}
	infile.close();
	std::vector<std::string> vlist;
	boost::split(vlist,body,boost::is_any_of("\n"));
	for(const auto& l : vlist)
	{
		size_t pos = l.find(" ");
		if(pos == std::string::npos)
		{
			std::cout << "GetListConf split error" << std::endl;
			continue;
		}
		std::string key = l.substr(0, pos);
		std::string val = l.substr(pos + 1);
		file_list_[key] = val;
	}
	return true;
}
bool CompressServer::GetNormalFile(std::string& filename, std::string& body)
{
	std::cout << "GetNormalFile ==== " << filename << " ===== "<< std::endl;
	if (!bf::exists(filename)) {
		std::cerr << "GetNormalFile --- file:"<<filename<<" not exists\n";
		return false;
	}
	int64_t fsize = bf::file_size(filename.c_str());
	body.resize(fsize);
	int fd = open(filename.c_str(), O_RDONLY);

	if(fd < 0)
	{
		std::cout << "GetNormalFile open error" << std::endl;
		return false;
	}

	flock(fd, LOCK_SH);
	int ret  = read(fd, &body[0], fsize);
	flock(fd,LOCK_UN);
	if(ret != fsize)
	{
		std::cout << "GetNormalFile read error" << std::endl;
		close(fd);
		return false;
	}
	close(fd);
	return true;
}
bool CompressServer::AddFileConf(std::string& file, const std::string &gzname)
{
	pthread_rwlock_wrlock(&rwlock_);
	std::string filepath = "mroot/list/" + file;
	file_list_[filepath] = gzname;
	pthread_rwlock_unlock(&rwlock_);
}

bool CompressServer::GetGzpFile(std::string& name, std::string& gzname)
{
	pthread_rwlock_rdlock(&rwlock_);
	auto fit = file_list_.find(name);
	if(fit == file_list_.end())
	{
		std::cout << "GetGzpFile find error" << std::endl;
		pthread_rwlock_unlock(&rwlock_);
		return false;
	}
	pthread_rwlock_unlock(&rwlock_);
	gzname = fit->second;
	return true;
}

bool CompressServer::SetListConf()
{
	std::stringstream tmp;
	for(const auto& e : file_list_)
	{
		tmp << e.first << " " << e.second << "\n";
	}
	std::ofstream ofile(COMMPRESS_FILE_CONF, std::ios::binary | std::ios::trunc);
	if(!ofile.is_open())
	{
		std::cout << "COMMPRESS_FILE_CONF open error" << std::endl;
	}
	ofile.write(tmp.str().c_str(), tmp.str().size());
	if(!ofile.good())
	{
		std::cout << "COMMPRESS_FILE_CONF write error" << std::endl;
		return false;
	}
	ofile.close();
	return true;
}
 
bool CompressServer::DirCheck()
{
	if(!bf::exists(GUNZIP_DIR))
	{
		bf::create_directory(GUNZIP_DIR);
	}
	std::string cmd = "bash transname";
	FILE* fd = popen(cmd.c_str(), "r");
	pclose(fd);
	sleep(2);

	std::cout << " Trans Name Finished" << std::endl;
	bf::directory_iterator dit_begin(GUNZIP_DIR);
	bf::directory_iterator dit_end;

	for(; dit_begin != dit_end; ++dit_begin)
	{
		if(bf::is_directory(dit_begin->status()))
		{
			continue;
		}
		std::string file = dit_begin->path().filename().string();
		std::string path = dit_begin->path().string();
		std::string cmd = "bash transname " + path;
		popen(cmd.c_str(), "r");
		if(IsNeedCompress(path))
		{
			std::cout << file << " IsNeedCompress!" << std::endl;
			if(CompressFile(file))
			{
				std::cout << file << " compress successful" << std::endl;
				std::string gzname = file + ".gz";
				AddFileConf(file, gzname);
			}
		}
	}
	return true;
}

bool CompressServer::IsNeedCompress(std::string filename)
{
	struct stat st;
	if(stat(filename.c_str(), &st) < 0)
	{
		std::cout << "Get " << filename << " status error" << std::endl;
		return false;
	}
	time_t cur_time = time(NULL);
	time_t file_atimt = st.st_atime;
	if(cur_time - file_atimt  > HEAT_TIME)
	{
		return true;
	}
	return false;
}

bool CompressServer::CompressFile(std::string& filename)
{
	std::string cmd;
	cmd = "bash comp " + filename;
	FILE* fd = popen(cmd.c_str(), "r");
	pclose(fd);
	return true;
}

bool CompressServer::UnCommpressFile(std::string& filename)
{
	std::string cmd;
	std::string file = filename.substr(0,filename.size() - 3);
	cmd = cmd = "bash uncomp " + file;
	FILE* fd = popen(cmd.c_str(), "r");
	pclose(fd);
	sleep(5);
	if(bf::exists(file))
	{
		std::cout << file  << "exist===========================" << std::endl;
	}
	else
	{
		std::cout << file << "NOT   exist===========================" << std::endl;
	}
	return true;
}
