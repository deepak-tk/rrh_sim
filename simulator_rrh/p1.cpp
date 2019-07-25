#include <iostream>
#include <string>
#include <unistd.h>

//using namespace std::chrono;

#include <fstream>
#include <chrono>
void logReqMsg( const std::string &text )
{
    std::chrono::milliseconds ms = std::chrono::duration_cast
	    < std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
    std::string stamp = std::to_string(ms.count()) ;
    std::string LogFilePath = getenv("OWN_IP_ADDR") + "_" + stamp;

    std::ofstream log_file(LogFilePath, std::ios_base::out | std::ios_base::app );
    log_file << text << std::endl;
}

int main()
{
	logReqMsg("1234");
	sleep(2);
	logReqMsg("5678");
}
