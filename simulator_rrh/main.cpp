#include<iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>
#include<stdio.h>
#include <dirent.h>
#include <map>
#include <time.h>
#include <cstring>
#include <list>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>

#include"SoapParser.hpp"
#define BUFFER 8192
using namespace RP1Broker;

enum msgType{ reset, log, retrieveParameter, modifyParameter};
std::map<std::string, msgType> mapMsgType ;
std::string xmlDir = "./resp"; 
struct alarmConfigParam
{
    int notificationCount;
    int notificationDelay;
};

std::string readFile(std::string& filename) {
    int fd;
    fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0)
    {
            throw std::runtime_error("readFile::Could not open file");
    }
    char buff[BUFFER] = {'\0'};
    read(fd, buff, BUFFER);
    close(fd);
    return buff;
}

void getFilePath(std::string& filePath, const std::string msgType,std::list<std::string>& msgMoList) 
{
    if (!msgMoList.empty())
    {
    	std::string className;
	className = msgMoList.front();
	filePath= xmlDir + "/" + msgType + "/" + className + ".xml";
	return;
    }

    filePath= xmlDir + "/" + msgType + ".xml";
    return;
}

void readAlarmConfigFile(alarmConfigParam &configParam)
{
    std::string  AlarmConfigFilePath = xmlDir + "/modifyParameterReq/alarmConfigFile.txt", str;
    std::ifstream in(AlarmConfigFilePath.c_str());
    std::vector<std::string> notificationval;
    if(!in)
    {
	throw std::runtime_error("alarmConfigFile:: Cannot open the File : " + AlarmConfigFilePath) ;
    }

    while (std::getline(in, str))
    {
	if(str.size() > 0 )
	{
	    notificationval.push_back(str.substr(str.find("=")+1));
        }
	else
	{
	    throw std::runtime_error("alarmConfigFile:: wrong file format :" ) ;
	}
    }
    configParam.notificationCount=stoi(notificationval[0]);
    configParam.notificationDelay=stoi(notificationval[1]);
    std::cout <<"notificationCount"<<configParam.notificationCount<<std::endl;
    std::cout<<"notificationDelay"<<configParam.notificationDelay<<std::endl;
}


void handleAlarmNotification(std::string& msgParamName,std::string& msgNewVal)
{
    if(msgParamName == "subscriber" and  msgNewVal == "TRUE")
    {
	int alarmNotiSentCount=0;
	std::string alarmNotificationFilePath=xmlDir + "/modifyParameterReq/ModuleFM.xml";

	std::string  alarmNotificationMsg=readFile(alarmNotificationFilePath);
	alarmConfigParam configParam;
	readAlarmConfigFile(configParam);

	while(alarmNotiSentCount < configParam.notificationCount)
	{
	    time_t my_time = time(NULL);
	    std::cout<<"Time : "<< ctime(&my_time)<< std::endl;
	    std::cout << "sending  Alarm Notification to FHGW....." << alarmNotificationMsg<< std::endl;
	    std::cout << "================================================" << alarmNotiSentCount<< std::endl ;
	    //udpWrapper.send(alarmNotificationMsg, peerIp, peerPort);
	    std::cout << "recv ack from FHGW" << std::endl;
	    //std::cout << reciever(udpWrapper) << std::endl;
	    sleep(configParam.notificationDelay);

	    alarmNotiSentCount ++;
	}
    }
}

int main()
{
    std::string soapReqMsg, msgType, msgId, soapRespMsg, SoapReqXmlFile, fileName, 
	RespFilePath, className,msgParamName,msgNewVal; 
    std::list<std::string> msgMoList;
    mapMsgType["modifyStateReq"] = reset;
    mapMsgType["techLogFileReq"] = log;
    mapMsgType["retrieveParameterReq"] = retrieveParameter;
    mapMsgType["modifyParameterReq"] = modifyParameter;

    DIR *d;
    struct dirent *dir;
    d = opendir("./req");
    if (d)
    {
	try{
	    while ((dir = readdir(d)) != NULL)
	    {
		if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 )
		    continue;
		std::cout<<std::endl<<" FIlE : "<<dir->d_name <<std::endl;
		fileName = dir->d_name;

		SoapReqXmlFile = "./req/" + fileName; 
		soapReqMsg=readFile(SoapReqXmlFile);

		SoapParser parser(soapReqMsg);
		parser.getMsgId(msgId);
		parser.getMsgType(msgType);

		std::cout<<std::endl<<"MsgId= "<<msgId<<std::endl<<"msgType= "<<msgType<<std::endl;
		std::cout<<std::endl<<"msgParamName="<<msgParamName<<std::endl<<"msgNewVal"<<msgNewVal<<std::endl;

		switch(mapMsgType[msgType])
		{
		    case reset: 
			std::cout<<"\n\n======no resp======\n"<<std::endl;
			break;
		    default:
			parser.getMsgMoList(msgMoList);
			getFilePath(RespFilePath, msgType, msgMoList);
			std::cout<<"\nFilePath : \n"<<RespFilePath<<std::endl;
			parser.addRelatesToSoapMsg(RespFilePath ,msgId,soapRespMsg);
			std::cout<<"\n\n======Resp xml======\n\n"<<soapRespMsg<<std::endl;
			if (mapMsgType[msgType] == modifyParameter && className == "ModuleFM")
			{
				parser.getMsgParamName(msgParamName);
				parser.getMsgNewVal(msgNewVal);
				handleAlarmNotification(msgParamName, msgNewVal);
			}
			break;
		}
	    }
	    closedir(d);
	}
	catch(std::exception &e) {
	    std::cout << e.what() << std::endl;
	}
    }
}
