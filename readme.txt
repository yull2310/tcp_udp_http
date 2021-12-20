1. 执行 make 生成 myServer 和 client 可执行程序
2. 配置文件 HostInfo.cfg 中的配置
	1）当需要运行 myServer 时，需要根据环境修改 Role-Server 下的相关配置，
	2）当需要运行 client 时，需要根据环境修改 Role-Client 下的相关配置
	其中TcpPort需要根据测试规则使用的端口来定；当测试tcp规则时，可以使用默认的20000端口
	HttpPort 目前和 TcpPort 的作用是一样的
	modbus规则使用端口：502
	ftp规则使用端口：21
	smb规则使用端口：139 或 445