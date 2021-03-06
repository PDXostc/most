/* Copyright (C) 2014 Jaguar Land Rover - All Rights Reserved
*
* Proprietary and confidential
* Unauthorized copying of this file, via any medium, is strictly prohibited
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
* Filename:	 Optolyzer.cpp
* Version:              1.0
* Date:                 Feb. 2014
* Project:              
* Contributors:         
*                       
*
* Incoming Code:        
*
*/
/**
*	Classes for the Optolyzer hardware; connect to, initialize and manage the Optolyzer
*	box, and through it provide acess to the MOST hardware (audio).
*/
#include "Optolyzer.h"


#include <stdexcept>

#include "fcntl.h"
#include <termios.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <syslog.h>

namespace DeviceAPI
{
namespace Most
{

void mostInitServer();

using namespace std;

	const int DELAY1=35000;
	const int DELAY2=100000;

	const int FIFO_Msg_Sz=81;

	// Static member that will hold the initialization commands to send to the MOST. Usually
	// set to the OptolyerInitCmds values below.
	std::vector<StringAndDelay> OptolyzerImpl::cmds;

	// Commands that are sent to the Optolyzer to initialize it and make it ready to send MOST command on to
	// MOST devices.
	vector<StringAndDelay> OptolyerInitCmds = {
	//TODO: +1001 is an unknown Optolyzer cmd, and it would seem to be too soon to be sending MOST
	// commands, so tried removing this. Everything still seems to work OK.
	/*	 "+970200\r\n" , "+9703\r\n" , "+980000\r\n" , "+1001\r\n" , "+980000\r\n" , "+22\r\n" , "+B4\r\n" , */
	{"+970200\r\n", DELAY2} ,
	{"+9703\r\n" , DELAY2},
	{"+980000\r\n" , DELAY2} ,
	{"+980000\r\n" , DELAY2} ,
	{"+22\r\n" , DELAY2} ,
	{"+B4\r\n" , DELAY2} ,
	//TODO:  +1000 is an unknown Optolyzer cmd, and it would seem to be too soon to be sending MOST
	// commands, so tried removing this. Everything still seems to work OK.
	/*	 "+B5\r\n" , "+B50101\r\n" , "+B50201\r\n" , "+1000\r\n" , "+B5\r\n" , "+5A0000000100001\r\n" , "+45\r\n" , */
	{"+B5\r\n" , DELAY2} ,
	{"+B50101\r\n" , DELAY2} ,
	{"+B50201\r\n" , DELAY2} ,
	{"+B5\r\n" , DELAY1} ,
	{"+5A0000000100001\r\n" , DELAY2} ,
	{"+45\r\n" , DELAY1} ,
	{"+98\r\n" ,DELAY1},
	{"+9803E8\r\n" , DELAY2} ,
	{"+4C0010\r\n"  , DELAY2} ,
	{"+4C1010\r\n" , DELAY2} ,
	{"+4C2010\r\n" , DELAY2} ,
	{"+4C3010\r\n" , DELAY2} ,
	{"+4C4501\r\n" , DELAY2} ,
	{"+4C5501\r\n" , DELAY2} ,
	{"+4C5701\r\n" , DELAY2} ,
	{"+42\r\n" , DELAY1} ,
	{"+89408701\r\n" , DELAY2} ,
	{"+AD\r\n" , DELAY1} ,
	{"+22\r\n" , DELAY1} ,
	{"+41\r\n" , DELAY1} ,
	{"+44\r\n" , DELAY1} ,
	{"+80\r\n" , DELAY1} ,
	{"+4F\r\n" , DELAY1} ,
	{"+4E0108\r\n" , DELAY2} ,
	{"+50\r\n" , DELAY1} ,
	{"+52\r\n" , DELAY1} ,
	{"+84\r\n" , DELAY1} ,
	{"+AA\r\n" , DELAY1} ,
	{"+AB\r\n" , DELAY1} ,
	{"+99\r\n" , DELAY1} ,
	{"+49\r\n" , DELAY1} ,
	{"+B50101\r\n" , DELAY2} ,
	{"+B50201\r\n" , DELAY2} ,
	{"+8006\r\n" , DELAY2} ,
	{"+AD0C\r\n" , DELAY2} ,
	{"+4F00\r\n" , DELAY2} ,
	{"+4D010000000000\r\n" , DELAY2} ,
	{"+5000\r\n" , DELAY2} ,
	{"+AB01\r\n" , DELAY2} ,
	{"+5900\r\n" , DELAY2} ,
	{"+89408701\r\n" , DELAY2} ,
	{"+AD\r\n" , DELAY1} ,
	{"+AA\r\n" , DELAY1} ,
	{"+49\r\n", DELAY1} ,
	{"+AA\r\n", DELAY1} ,
	{"+A900\r\n", DELAY1} ,
	{"+A804\r\n", DELAY1} ,
	{"+8B40000042\r\n", DELAY2} ,
	{"+8C40000001\r\n", DELAY2} ,
	{"+8B4000014A\r\n", DELAY2} ,
	{"+8C40000101\r\n", DELAY2} ,
	{"+8B40000262\r\n", DELAY2} ,
	{"+8C40000201\r\n", DELAY2} ,
	{"+8B4000036A\r\n", DELAY2} ,
	{"+8C40000301\r\n", DELAY2} 
	 };

	// Container of user callback functions.
	std::vector<OptolyzerRecvCB*> OptolyzerImpl::cbList;

	OptolyzerImpl* OptolyzerImpl::_instance;

	bool loggingEnabled=false;

/** Singleton; you can call this from any scope.
*   To access the created OptolyzerImpl you must call instance() to obtain a reference.
*/
static const char* configName = "/etc/most/conf";

/** getConfig: Read config info from /etc/most/conf, such as which serial port to use.
 *	The rules for conf file content and usage are:
		0 or more MOST_SERIAL <device path> entries, one per line.
		If <device path> can be opened, it will be used as the serial port for MOST, regardless of whether  this port is connect to the Optolyzer or not.
		If <device path> cannot be opened, the next ‘MOST_SERIAL <device path>’ is parsed and its device path is tried.
		If none of the ‘MOST_SERIAL <device path>’ entries specify a device path that can be opened, the default /dev/ttyS0 is used.
 *
 */
string OptolyzerImpl::getConfig(enum ConfigTags tag)
{
	 ifstream  cf(configName);
	 string defaultPort="/dev/ttyS0";

	 if(!cf)
	 {
		 syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Cannot open config file %s; will use default port %s", configName, defaultPort.c_str() );
		 return defaultPort;
	 }

	 string tagName, tagVal;
	 int fd = -1;
	do
	{
	cf >> tagName >> tagVal;
	syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Tag %s and value %s seen in config file:", tagName.c_str(), tagVal.c_str());

	 switch(tag)
	 {
	 	 case SERIAL_PORT:

	 			if((fd=open(tagVal.c_str(), O_RDWR|O_NOCTTY|O_SYNC|O_NONBLOCK)) < 0 )
	 			{
	 				syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Can't open %s", tagVal.c_str());
	 				tagName = "";
	 				break;
	 			}


	 		 if(tagName == "MOST_SERIAL")
	 		 {
	 			 close(fd);
	 			 return tagVal;
	 		 }
	 		 break;
	 	 case LOGGING:

 			syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Tag %s and value %s seen in config file:", tagName.c_str(), tagVal.c_str());
	 		 if(tagName == "LOGGING")
	 		 {
	 			 loggingEnabled = (tagVal == "1") ? true : false;
	 			 return "";
	 		 }
 			break;

	 	 default:
	 		syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Did not see tag %d", tag);
	 		break;
	 }

	} while( !cf.eof() );


			return defaultPort;
}

/** create: creates the OptolyzerImpl singleton and initializes hardware if necessary.
 * 	serialDev: 	path to port the Optolyzer is connected to.
 * 	initCmds:  	if not empty, these commands will be sent to initialize the Optolyzer/MOST hardware.
 * 				otherwise, the default commands in OptolyerInitCmds above will be used.
 *
 */
void OptolyzerImpl::create(std::string& serialDev, std::vector<StringAndDelay>& initCmds)
{

	umask(0); // Necessary, otherwise code that uses the MOST plug in will fail to open.

    // Singleton; only need one instance.
	if(!_instance )
	{
		_instance = new OptolyzerImpl(serialDev,initCmds);
	}
}
/**
 *	Ctor; requires a path to the serial port that the Optolyzer is connected to.
 *
 *	Uses RAII; if all resources are not acquired and initialized, this ctor will fail and no
 *	OptolyzerImpl object will be created.
 *
 *	TODO: initCmds parameter to be removed, as it is now used only by mostInitServer,
 */
OptolyzerImpl::OptolyzerImpl(std::string& serialDev, std::vector<StringAndDelay>& initCmds) :
		serialPort(-1), afterCmdWait(200000)
{

		if( (serialPort = open(serialDev.c_str(), O_RDWR|O_NOCTTY|O_SYNC|O_NONBLOCK)) < 0 )
		{
			syslog(LOG_USER | LOG_DEBUG, "MOSTEXT: Could not open serial port; no MOST communication will be possible.");
			return;
		//	throw CtorFails {};
		}

		// INVARIANT: only get here if port was opened successfully.
  
		// Setup tty speed and modes:
		struct termios settings;
		settings.c_iflag = settings.c_oflag = settings.c_cflag = settings.c_lflag = 0;
		int stat = tcgetattr(serialPort, &settings);

		// Set to 115200 baud:
		settings.c_cflag &= 07777760;
		settings.c_cflag |= 00010002;

		// Clear ixon:
		settings.c_iflag &= ~IXON;

		// set inlcr:
		settings.c_iflag &= ~INLCR;
		settings.c_iflag |= INLCR;

		// Clear echo:
		settings.c_lflag &= ~ECHO;

		// Clear igncr:
		settings.c_iflag &= ~IGNCR;

		tcsetattr(serialPort, TCSANOW, &settings);	
		if( stat )
		{
			throw CtorFails {};
		}
}
/**
* Send a command string to the Optolyzer hardware. The optional wait param sets a delay
* in microseconds for use after sending the command; this delay is invoked after
* the command is sent, before returning. A wait of 0 causes the default wait time to be used.
*/
int OptolyzerImpl::send(std::string& cmd, unsigned int wait)
{
	int nb=0;

	if( serialPort > 0 )
	{
		nb=write(serialPort, (void*)(cmd.c_str()), cmd.size());

		usleep( wait == 0 ? afterCmdWait : wait);
	}
	return nb;
}

/**
* Receive a status string from the Optolyzer hardware. The optional wait param sets a time
* to wait for a string before giving up; 0 means wait forever.
* eolDef defines the character that is expected to mark the end of every status string.
*/
int OptolyzerImpl::recv(std::string& recvStr, unsigned int wait, int eolDef)
{
        int len, cnt=0;
		char c;
		const int EOFCNT=100;
		int eofcnt=EOFCNT;

		if( serialPort <= 0 )
			return 0;

		do {

			len = read(serialPort, (void*)(&c), 1);
			if( len != 1 )
			{
				if( --eofcnt <= 0 )
					return len;
				usleep(wait);
				continue;
			}

			eofcnt=EOFCNT;
			cnt++;

			recvStr.append(1, c);

		} while(c != eolDef);
		
		return cnt;
}

/**
* Add a callback to be invoked when a new string is received from the Optolyzer.
*/
void OptolyzerImpl::addRecvCB(OptolyzerRecvCB& cbObj)
{
	cbList.push_back(&cbObj);
}

void OptolyzerImpl::removeRecvCB(OptolyzerRecvCB& cbObj)
{
	// STL erase-remove idiom; see Meyers "Effective STL".
	cbList.erase( remove(cbList.begin(), cbList.end(), &cbObj), cbList.end());

}
/**
* Set the strings that applyInitCmds will send to the Optolyzer.
*/
void OptolyzerImpl::setInitCmds(std::vector<StringAndDelay>& _cmds)
{
	cmds = _cmds.size() ? _cmds : OptolyerInitCmds;

}
/**
* Send the ASCII strings in _cmds to the Optolyzer.
*/
void OptolyzerImpl::applyInitCmds(void)
{
	for_each(cmds.begin(), cmds.end(),
		[this](StringAndDelay sd) { send(sd.str, sd.delay); } // this gives access to afterCmdWait.
	);
}

OptolyzerImpl::~OptolyzerImpl() 
{
	if( serialPort >= 0 )
		close(serialPort);
}

/**
 * This function gets called from a child process by any instance of OptolyzerImpl that detects that the mostInitServer
 * process is not currently running.
 *
 * Create the named pipes, open them, send the init. commands to the Optolyzer, and then send out "OK" on one pipe
 * when init is done.
 */
void mostInitServer()
{
	// TODO: remove

}

} // Most
} // DeviceAPI

