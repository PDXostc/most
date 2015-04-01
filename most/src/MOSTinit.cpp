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
* Filename:	 MOSTinit.cpp
* Version:              1.0
* Date:                 Apr 1. 2015
* Project:              
* Contributors:         
*                       
*
* Incoming Code:        
*
*/
/**
*	This needs to be run before any POC that uses MOST audio can be run; the intent is to
*	start it via systemd, but it can also be run from the command line at need.
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

/**
 * Opens the serial port, sends to the Optolyzer its initialization strings, then exits.
 * This allows instances of the MOST XW extension to communicate w/o having to be concerned with
 * initializing the serial port or MOST.
 */
int main(int argc, char** argv)
{
	using namespace::DeviceAPI::Most;
	vector<StringAndDelay> cmds;

	cout << "Starting MOSTinit\n";
	   syslog(LOG_USER | LOG_DEBUG, "MOSTINIT: Starting MOSTinit");

	// Find serial port; either default ttyS0, or the specified in /etc/most/conf
	string config = OptolyzerImpl::getConfig(SERIAL_PORT);

	// Set member to reference the list of Optolyer init. strings.
	OptolyzerImpl::setInitCmds(cmds);

	OptolyzerImpl::create(config, cmds);


	OptolyzerImpl& oi =  OptolyzerImpl::instance();
	oi.applyInitCmds();

	cout << "MOSTinit complete\n";
	syslog(LOG_USER | LOG_DEBUG, "MOSTINIT: MOSTinit complete.");


	return 0;
}


