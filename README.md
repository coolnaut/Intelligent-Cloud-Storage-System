# PrivateSmartCloudBackupSystem
## ————私人智能云备份系统

### 简介
本项目实现功能为：对本地备份文件检测，对于新文件或者修改的文件实现自动备份到私人服务器上，可以通过网页浏览服务器，进行下载。
### 用法
项目实现了客户端和服务器，客户端可布置在Windows下，服务器布置到私人服务器上(服务器是在CentOS7开发)。

#### 配置环境
1. 当把客户端clone到Windows下后，还需要配置VS环境进行编译，首先是openssl与boost。
  openssl是OpenSSL-Win64
  boost是boost_1_70_0-msvc-14.1-64
 上述两个文件都是exe直接执行后会生成相应的库文件。相应配置见下图：
![](https://github.com/Be-doing/PrivateSmartCloudBackupSystem/blob/master/Irrelevant_picture/%E7%8E%AF%E5%A2%83%E9%85%8D%E7%BD%AE.png)
2. 在cloudclient.h文件中声明了如下参数：
 ```
 #define BACKUP_PATH "back"		//文件监控与备份目录
#define BACKUP_INFO "backupinfo.txt"		//文件备份信息文件
#define RANGE_MAX_SIZE (10<<20)
#define SERVER_IP "192.168.138.135"//服务器IP地址
#define SERVER_PORT 9090//服务器程序运行端口
#define BACKUP_URI "/list/"//服务器备份文件目录
 ```
![](https://github.com/Be-doing/PrivateSmartCloudBackupSystem/blob/master/Irrelevant_picture/%E5%8F%82%E6%95%B0%E8%AF%B4%E6%98%8E.png)
3. 服务器程序是在centos7环境下开发，直接clone后，可直接执行该程序main2。
4. 修改相应参数后，重新make会生成main执行文件。
服务器程序(server.h)
![](https://github.com/Be-doing/PrivateSmartCloudBackupSystem/blob/master/Irrelevant_picture/%E6%9C%8D%E5%8A%A1%E5%99%A8%E7%A8%8B%E5%BA%8F%E5%8F%82%E6%95%B0%E8%AE%BE%E7%BD%AE.png)
压缩程序(compressserver.h)
![](https://github.com/Be-doing/PrivateSmartCloudBackupSystem/blob/master/Irrelevant_picture/%E5%8E%8B%E7%BC%A9%E7%A8%8B%E5%BA%8F%E5%8F%82%E6%95%B0.png)
 ### 示例
 #### 客户端备份
 首先是备份下目录的文件
 ![](https://github.com/Be-doing/PrivateSmartCloudBackupSystem/blob/master/Irrelevant_picture/backdir.png)
 #### 浏览器访问
 
 #### 服务器压缩服务
