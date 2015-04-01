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
* Filename:	 Optolyzer.h
* Version:              1.0
* Date:                 Feb. 2014
* Project:              
* Contributors:         
*                       
*
* Incoming Code:        
*
*/

/*
 * Provide an interface to the Optolyzer hardware that send and receives strings.
 * Receiving strings from the Optolyzer occurs in a separate thread, that will also
 * invoke any callback registered, passing along the received string.
 *
 * Handles Singleton management and detection of the need for one time (per power cycle)
 * initialization of the Optolyzer.
 */

#ifndef OPTOLYZER_H
#define OPTOLYZER_H

#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

namespace DeviceAPI {
namespace Most {

struct StringAndDelay
{
	string str;
	int delay;  // In microseconds.
};

enum ConfigTags { SERIAL_PORT, LOGGING };

class OptolyzerRecvCB;

/**
*	\brief This class provides an interface to the Optolyzer hardware
*	that initializes the Optolyzer
*   by sending a default set of commands (overridable); provides a send command capability, and
*	a receive status capability that can be tied to a callback function.
*/
class OptolyzerImpl
{
	public:

	/** Exception object definitions:
	*/
	struct CtorFails {};

	/** Read configuration info from /etc/most/conf
	*/
	static string getConfig(enum ConfigTags);

	/** Singleton; you can call this from any scope.
	*   To access the created OptolyzerImpl you must call instance() to obtain a reference.
	*/
	static void create(std::string& serialDev, std::vector<StringAndDelay>& initCmds);

	/** The only way to access an OptolyzerImpl object.
	*/
	static OptolyzerImpl& instance(void)
	{
		return *(_instance);
	}

	// Operations:

	/**
	* Send a command string to the Optolyzer hardware. The optional wait param sets a delay
	* in microseconds that occurs after sending the command.
	* A wait of 0 causes the default wait time to be used.
	*/
    int send(std::string& cmd, unsigned int wait=0 );

	/**
	* Receive a command string from the Optolyzer hardware.
	* Optional wait not implemented yet; will set a timeout for return if no
	* command is received within the specified waittime.
	* eolDef defines the character that must
	* be at the end of an incoming status string in order for it to be
	* recognized as complete.
	*
	* Receipt of a command will call any callback registered with addRecvCB.
	*/
	int recv(std::string& recvStr, unsigned int wait=0, int eolDef='\n');

	/**
	* Getter/setter for the default wait time for use by send().
	*/
	unsigned int getWait(void) { return afterCmdWait; }
	void setWait(unsigned int wait) { afterCmdWait = wait; }

	int getPort(void) { return serialPort; }

	/**
	* Static method that can be called before create() to set a callback that will be invoked
	* each time a command is received from the Optolyzer HW.
	*/
	static void addRecvCB(OptolyzerRecvCB& cbObj); 

	static void removeRecvCB(OptolyzerRecvCB& cbObj);

	/**
	* Set the list of init. commands to be sent to Optolyzer/MOST.
	*/
	static void setInitCmds(std::vector<StringAndDelay>& _cmds);

	/**
	* Send the commands set by setInitCmds.
	*/
	void applyInitCmds(void);

	// Set by create() when ctor returns; accessed via instance() with possible blocking
	// by join call on readyThrd.
	static OptolyzerImpl* _instance;  // TODO: should be private; public now to allow server access.

private:

	// Only called by create()
	OptolyzerImpl(std::string& serialDev, std::vector<StringAndDelay>& initCmds);
	~OptolyzerImpl();

	// The /dev/ttyXXX port the Optolyzer HW is connected to.
	int serialPort;
	unsigned int afterCmdWait;  // Delay after sending serial cmd, in usec.

	// Container for receive callbacks.
	static std::vector<OptolyzerRecvCB*> cbList;

	// Collection of initialization strings sent to the  Optolyzer hardware
	// once on startup.
	static std::vector<StringAndDelay> cmds;

	// No copy or assign allowed, nor default ctor.
	OptolyzerImpl(OptolyzerImpl&);
	OptolyzerImpl& operator=(OptolyzerImpl&);
	OptolyzerImpl();

};

/**
*
*  \brief lass for user defined callback to be invoked when a string is received from the MOST/Optolyzer hardware.
*/
class OptolyzerRecvCB
{
	public:

	OptolyzerRecvCB(){}

	/** User must implement this with their own class derived from OptolyzerRecvCB.
	* When a string is recv'd from the Optolyzer, it is returned in recvStr.
	* TODO: status and data are placeholders for now.
	*/
	virtual void userCB(string&, int, void* data=0)=0; //{if(data == 0) data=0;};

	/**
	* This will allow conditional calling of the receive callback depending on the filter criteria.
	*/
	virtual bool filter( string& )
		{
			return false; // Do not filter.
		}

	private:
	OptolyzerRecvCB(const OptolyzerRecvCB&);
	OptolyzerRecvCB& operator=(const OptolyzerRecvCB&);

};

} // Most
} // DeviceAPI

#endif // OPTOLYZER_H
