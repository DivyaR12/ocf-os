ATIS OS-IoT OneM2M/OCF BRIDGE DEMO
==================================


INTRODUCTION

This repository contains a demo of an interworking function between the oneM2M and Open Connectivity Foundation (OCF) IoT standards. This type of function is called a "Bridge" in OCF and an Iterworking Proxy Entity (IPE) in oneM2M standards. The bridge was developed as part of the ATIS Open Source IoT (OS-IoT) project - www.os-iot.org

The bridge is intended to be of demonstration quality only and is not intended for practical use in real applications. However, it could be a suitable basis for developing a commercial application with further development.

For important licensing information please see the LICENSE.md file included in this repository.


SUPPORTED FUNCTIONALITY

The bridge has a unidirectional capability which can expose OCF devices (i.e. OCF servers) to the oneM2M ecosystem. The bridge presents itself to the OCF environment as an OCF controller (i.e. an OCF client) to control the devices exposed in oneM2M. oneM2M resources are created that correspond to discovered OCF devices. Currently the only device type supported are simple deviceLights, though this demo could be extended to support other device types.

In the demo, OCF device discovery is only performed when the bridge is first started. OCF devices that are added (or removed) after the bridge has been started will not be detected. Therefore, it is important that any OCF devices are started before the bridge.

The bridge uses the standard OCF device model for a light, and the corresponding oneM2M Home Appliance Information Model (HAIM) resource structure. Alignment between OCF and oneM2M in the device models means that it is possible to support full interworking between the standards.

The bridge supports IoTivity "just works" security. The bridge may also use PSK oneM2M security.


DEPENDENCIES

The bridge and other applications in this repository should be compatible with a wide range of Linux devices. The have been tested on Ubuntu PC (x64), Raspbian and Linaro operating systems.

The bridge has dependencies on:
- the ATIS OS-IoT oneM2M library
- the IoTivity version 1.3.1 OCF library
The installation and build instructions provide guidance on how to install these.

The demo also requires a separate oneM2M CSE to serve as a database for the oneM2M system. The demo has been tested on using Eclipse OM2M version 1.3.0 and Interdigital Chordant CSEs.

NOTE: If using OM2M then the default IP port number used for COAP in OM2M must be modified if the CSE is co-hosted with any OCF functions in order to avoid conflicts with ports used by OCF. This can be done by editing the OM2M file "configuration/config.ini" to set the port as follows: "org.eclipse.om2m.coap.port=15683".


DEMO ARCHITECTURE

At least the following are required to perform the demo:
1) A simulated (or real) OCF device light that complies with the OCF light specification. ("server" directory in this repository).
2) The OS-IoT bridge function (root directory in this repository)
3) A oneM2M CSE - see notes on dependencies above
4) A oneM2M AE that can modify the light settings in the CSE ("onem2mControlAE" directory in this repository)

In addition the following may be added:
5) An OCF control function (OCF client) that can control the light directly ("ocfControl" directory in this repository)
6) A web interface to the oneM2M and OCF control functions ("demoWebServer" directory in this repository)

These functions may be cohosted on the same computer, or may be distributed over multiple hosts. Note that the bridge and the oneM2M AE will require configuration of IP address information - see below.

For more information on the individual files in this repository see the file "FILES.TXT"


DEMO OPERATION

NOTES: 
- In the instructions below line starting ">" indicate commands intended to be typed by the user. Lines starting with # are comments.
- Items in angle brackets (e.g. <example>) indicate values that should be substituted with the specifics for your configuration.
- It is assumed that the repository is installed at the path "~/ocfbridgeclient". 
- Because the demo uses OCF security it is important that OCF security parameters are correctly configured. The default parameters in the repository are pre-configured to allow all the functions to communicate. If you replace any of the functions (e.g. to use a different OCF deviceLight server) then the security configuration files must be updated. See information in the FILES.txt document.
- The applications that support OCF interfaces must be run in the context of their directory in order to allow them to discover security configuration files. 


Use the installation and build instructions in the file "BUILD.txt" to build the repository on as many computers as you plan to use for the demo. By default, all applications are designed to run using the command line, so ensure you have command line access to all computers. Extensions to allow other ways of using the applications are explained below. In order to first test the demo, it is strongly recommended that you start by running all applications on the same computer in different termianl windows. Once you have this working you may then start to distribute applications to different computers.

1) Start the OCF light device application (OCF server):
> cd ~/ocfbridgeclient/server
> ./server

2) Verify that you can control the OCF light device using an OCF client. In another window:
> cd ~/ocfbridgeclient/ ocfControl
# Turn the light on
> ./ocfSwitch 1
# Turn the light off
> ./ocfSwitch 0
The light device will print output indicating the changes to the light state.

3) In a new window, start the oneM2M CSE taking care that is is configured to avoid conflicts with the default COAP and COAPS IP port numbers used by IoTivity. For example, if using OM2M:
> cd <OM2M directory>/in-cse
> sudo ./start.sh

4) Start the OCF bridge in a new window:
> cd ~/ocfbridgeclient
> ./brige <OPTIONS>
If you are using the OM2M CSE and both the CSE and the bridge are running on local host, then the <OPTIONS> for the bridge command may be omitted. If not, then the following options should be specified for your situation:

-h <HOST IP ADDRESS AND PORT FOR THE CSE>
Use this to specify the address the bridge should use to contact the CSE. e.g.:
-h 192.168.0.10:8080
(Default value is 127.0.0.1:8080)

-a <CSE ROOT ADDRESS>
Use this to specify the CSE root address. e.g.:
OneMPOWER-IN-CSE
(Default value is /in-cse/in-name)

-f <AE ID>
Use this to specify the AE-ID of the bridge to the CSE. e.g.:
-f SAE01
(Default value is admin:admin)

--poaAddr <AE IP ADDRESS>
Use this to specify the address used by the CSE to send notifications to the bridge. It should be the IP address of the bridge. e.g.
--poaAddr 192.168.0.11
(Default value is 127.0.0.1)

--poaPort <PORT NUMBER>
Use this to specify the IP port number used by the CSE to send notifications to the bridge. e.g.
--poaPort 6666
(Default value is 18888)

--pskIdentity, --pskKeyAscii, --pskKey
Use these to specify the identity (--pskIdentity) and key (--pskKey or --pskKeyAscii) for PSK security to the CSE. Including these will activity oneM2M PSK security.

In addition to the configuration options above, there are two special options accepted by the bridge:

--deleteAe
This will delete any AE left on the CSE after an abmoral termination of the bridge. Use this command if an AE already on the CSE is blocking operation of the bridge.

--testOnly
This will stop the bridge application working as a bridge! If this option is specified the bridge will just to discovery of OCF devices.

Running this command will discover any OCF light devices and create an AE resource and HAIM resources for each device light. The bridge will then create subscriptions to monitor state changes to the device or the oneM2M resources.



5) Operate the light using the oneM2M interface. In another window:
> cd ~/ocfbridgeclient/onem2mControlAe
# Switch on
> ./switch.sh 1 <OPTIONS>
# Switch off
> ./switch.sh 0 <OPTIONS>

In this command the <OPTIONS> should be as above for the bridge command (-h, -f, --pskIdentity, --pskKeyAscii, --pskKey), also the option for setting the CSE root is "-A <CSE ROOT ADDRESS>" (NOTE this is capital "A" as opposed to the lower case "a" in the bridge).

These commands will send an update to the CSE which will change the light state in its resources. This will cause a notification to the bridge which will in turn update the OCF device state. The OCF light will print a change in state of the light.


DEMO EXTENSIONS

The following extensions are are possible for the demo:
1) The OCF light can control an actual light rather than just printing state to the output.
2) A web server can be used to trigger the OCF light control or oneM2M light control commands.

* Control of physical light

The repository includes code to allow a physical light to be controlled by the OCF light. Supported options are either a light attached to GPIO-C of a DragonBoard 410C or a Pimoroni Blinkt! attached to a Raspberry Pi.

To build the DragonBoard/GPIO option:
> cd ~/ocfbridgeclient/server
> make clean
> make GPIO=1


To build the Rapsberry Pi/Blinkt! option:
> cd ~/ocfbridgeclient/server
> make clean
> make EXTCMD=1
Note that this depends on Python scripts included in the demoWebServer directory, as well as the Blinkt! Python library provided by Pimoroni.

Following these builds, running the light server as described above will switch a suitably attached light as well as printing output to the terminal.

* Web Server

A simple Python web server application is provided in the "demoWebServer" directory. This can be run using Python 2.7:
> cd ~/ocfbridgeclient/demoWebServer
> python webserver.py

By default the web server runs on port 8899. When a command is selected on the web page the server will run the corresponding terminal command to change the light state using OCF or oneM2M control. These commands are configured in the directory "commands" in the script "webserver.py". This should be edited to add any parameters the commands require in your configuration.

