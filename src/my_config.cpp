#include "my_config.h"

CONF_INFO::CONF_INFO(){
	const char * file_path = "/home/linbird/workspace/Http-Server/cfg/config.json";
 	std::ifstream in(file_path, std::ios::binary);
	if(in.is_open()){ 
		Json::Value file_root;
		Json::Reader reader;  
		if(reader.parse(in,file_root))
		{
			port = file_root["server"]["port"].asInt();
			private_key = file_root["server"]["private_key"].asString();
			certificate = file_root["server"]["certificate"].asString();
			source_root  = file_root["source"]["root_path"].asString();
			data_root  = file_root["data"]["root_path"].asString();
			default_page = file_root["page"]["default"].asString();
			index_page = file_root["page"]["index"].asString();
			root = file_root["global"]["root"].asString();
			login = file_root["page"]["login"].asString();
			file = file_root["page"]["states"].asString();
			states = file_root["page"]["states"].asString();
			test = file_root["page"]["test"].asString();
			in.close();
			// std::cout << "成功读取配置文件：" << "port = " << port << std::endl
			// 	<< "private_key: " << private_key << std::endl
			// 	<< "certificate: " << certificate << std::endl
			// 	<< "source_root: " << source_root << std::endl
			// 	<< "data_root: " << data_root << std::endl
			// 	<< "default_page: " << default_page << std::endl
			// 	<< "index_page: " << index_page << std::endl
			// 	<< "root: " << root << std::endl
			// 	<< "login: " << login << std::endl
			// 	<< "file: " << file << std::endl
			// 	<< "states: " << states << std::endl;
		}else{
			in.close();
			std::cout << "parse error\n" << std::endl;
			exit(-1);
		}
 	}else{	
		std::cout << "Error opening config file " << file << std::endl; 
		exit(-1);
	}	
}
