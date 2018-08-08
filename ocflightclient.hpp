// ocflightclient.hpp
/* 
 *	SPDX-License-Identifier:	BSD-3-Clause 
 * The above license addresses copyright obligations. Please refer to Section 10.4 of the 
 * ATIS Operating Procedures for information about patent licensing obligations under 
 * ATIS' Intellectual Property Rights Policy.
 */

#ifndef OCFLIGHTCLIENT_H
#define OCFLIGHTCLIENT_H

#include <signal.h>
#include <thread>
#include <functional>
#include <string>
#include <iostream>
#include <memory>
#include <exception> 
#include <mutex>
#include <map>
#include <algorithm>

#include "ocstack.h"
#include "OCPlatform.h"
#include "OCApi.h"
#include "ocpayload.h"



using namespace OC;

namespace PH = std::placeholders;

typedef std::map <std::string, std::shared_ptr<OC::OCResource> > uriToResourceMapType;
typedef std::map <std::string, std::string> subRiToSidMapType;

struct ocDeviceType {
  std::string deviceName = "";
  bool isLight = false;
  bool isInCse = false;
  bool gotValue = false;
  bool value = false;
  uriToResourceMapType resources;
  std::string binarySwitchUri = "";
};

typedef std::map <std::string, ocDeviceType > sidToDeviceMapType;

enum findOperation { getBinarySwitch, postBinarySwitch };

extern sidToDeviceMapType foundDevices;
extern std::mutex foundDevicesMutex;
extern subRiToSidMapType subRiToSidMap;

void onPostSwitch(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode);

void postSwitchValue(std::shared_ptr<OC::OCResource> resource, bool value);

void getResBinarySwitch(std::shared_ptr<OC::OCResource> resource, std::string resourceSid);

void getOcfDeviceResource(std::shared_ptr<OC::OCResource> resource, std::string resourceSid);

void setOcfResourceHostCoaps(std::shared_ptr<OC::OCResource> resource);

void discoverOcfResources( findOperation op, bool value);

#endif

