Copyright (c) 2014, Intel Corporation, Jaguar Land Rover

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Name: MOST Crosswalk extension
Version: XW_TizenIVI3_0_01FEB_AGL_05MAR2015
Base Image: tizen-3.0-ivi_20150201.3(http://download.tizen.org/releases/milestone/tizen/ivi-3.0/tizen-3.0-ivi_20150201.3/images/atom/ivi-mbr-i586/)
Maintainer: Jeff Eastwood <jeastwo1@jaguarlandrover.com>
Mailing list: dev@lists.tizen.org

Overview: This extension interfaces the MOSTAudio Tizen application to a MOST amplifier.

Build Instructions: 

gbs build process:

	To build, use:

		 gbs build --spec most_extension.spec -A i586

	extension_common is a static library that must be linked into all
	extensions.

	extension_tools contains the tool to create the XWalk boilerplate code from
	the javascript template


KnownIssues: 

Apr. 7 2015:  The systemd mechanism does not correctly start the initMOST executable on startup, on the NUC. 
For NUC users, make sure you change permissions of the tty device for your USB to serial port adapter to a+rw, 
and run /usr/bin/initMOST before attempting to use Optolyzer/MOST hardware.

