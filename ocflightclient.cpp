// ocflightclient.cpp
/* 
 *	SPDX-License-Identifier:	BSD-3-Clause 
 * The above license addresses copyright obligations. Please refer to Section 10.4 of the 
 * ATIS Operating Procedures for information about patent licensing obligations under 
 * ATIS' Intellectual Property Rights Policy.
 */




#include "ocflightclient.hpp"


using namespace OC;

namespace PH = std::placeholders;



sidToDeviceMapType foundDevices;
std::mutex foundDevicesMutex;
subRiToSidMapType subRiToSidMap;




void onPostSwitch(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
  if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED || eCode == OC_STACK_RESOURCE_CHANGED)
    std::cout << "POST request was successful" << std::endl;
}

void postSwitchValue(std::shared_ptr<OC::OCResource> resource, bool value) {
  OCRepresentation rep;
  rep.setValue("value", value);
  resource->post(rep, QueryParamsMap(), &onPostSwitch);
}

void getResBinarySwitch(std::shared_ptr<OC::OCResource> resource, std::string resourceSid) {
  std::string resourceUri = resource->uri();
  auto getResBinarySwitchLambda = [resourceSid, resourceUri] (const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
    std::cout << "getResBinarySwitch Lambda!" << std::endl;
    if (eCode==OC_STACK_OK) {
      std::unique_lock<std::mutex> lock(foundDevicesMutex);
      if (foundDevices.find(resourceSid)!=foundDevices.end()  ) {
        foundDevices[resourceSid].gotValue = rep.getValue("value", foundDevices[resourceSid].value);
        std::cout<<"\tSet SID:"<<resourceSid<<" to value "<<foundDevices[resourceSid].value<< std::endl;
        foundDevices[resourceSid].binarySwitchUri= resourceUri;
        std::cout<<"\tSet binarySwitchUri to: "<< resourceUri <<std::endl;
      } else
        std::cout << "Can't match SID!. SID: " << resourceSid << std::endl;
    } else
      std::cout << "\t Stack error. eCode: "<< eCode << std::endl;
  };
  std::cout << "getOcfVerticalResource. URI: " << resourceUri << std::endl;
  QueryParamsMap test;
  resource->get(test, getResBinarySwitchLambda);
}



void getOcfDeviceResource(std::shared_ptr<OC::OCResource> resource, std::string resourceSid) {
 auto onGetLambda = [resourceSid] ( const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
   std::cout << "On Get Lambda!" << std::endl;
    if (eCode == OC_STACK_OK) {
      std::string name;
      rep.getValue("n", name);
      std::replace( name.begin(), name.end(), ' ', '_'); // Remove whitespace in names
      auto types = rep.getResourceTypes();
      for (auto & t : types)
        std::cout << "\tResource Type: "<< t << std::endl;
      {
        std::unique_lock<std::mutex> lock(foundDevicesMutex);
        if (foundDevices.find(resourceSid)!=foundDevices.end()) {
          bool uniqueName = true;
          for (auto & d : foundDevices)
            if (d.second.deviceName == name) {
              uniqueName=false;
              break;
          }
          if (uniqueName)
            foundDevices[resourceSid].deviceName = name;
          else // Add SID to create uniqueName
            foundDevices[resourceSid].deviceName = name +"_"+ resourceSid;
          foundDevices[resourceSid].isLight = std::find(types.begin(), types.end(), "oic.d.light") != types.end()
// *********************************************************************
// Very bad hack to work around problem on CTT tool;
          || name == "Device_1";
// *********************************************************************
           std::cout << "\tStored name: " << foundDevices[resourceSid].deviceName << 
                        " in foundDevices. With isLight: "
           << foundDevices[resourceSid].isLight << std::endl;
        } else
        std::cout << "\tError: found name for unknown SID: " << resourceSid << std::endl;
      }
    } else
       std::cout << "Stack error in onGet!" << std::endl;
  }; // End of onGetLambda
  QueryParamsMap test;
  resource->get(test, onGetLambda);
}

void setOcfResourceHostCoaps(std::shared_ptr<OC::OCResource> resource) {
  std::cout<<"Current host: "<< resource->host()<< std::endl;
  auto hosts = resource->getAllHosts();
  const std::string coap6url="coaps://[";
  const std::string coap4url="coaps://";
  std::string ip6coaps="";
  std::string ip4coaps="";
  for( auto h:hosts) {
    std::cout<<"\tHost: " << h << std::endl;
    if (h.compare(0, coap6url.length(), coap6url) == 0)
      ip6coaps=h;
    else if (h.compare(0, coap4url.length(), coap4url) == 0)
      ip4coaps=h;
  }/*
  if (!ip6coaps.empty()) {
    std::cout << "Setting host to: " << ip6coaps << std::endl;
    resource->setHost(ip6coaps);
  } else */
if (!ip4coaps.empty()) {
    std::cout << "\tSetting host to: " << ip4coaps << std::endl;
    resource->setHost(ip4coaps);
    std::cout <<"\tHost is now: "<< resource->host()<<std::endl;
    std::cout <<"\tTransport is now: "<<resource->connectivityType()<<std::endl;
  }
}

void discoverOcfResources( findOperation op, bool value) {
  auto foundResourceLambda = [op, value] (std::shared_ptr<OC::OCResource> resource) {
    std::cout << "Found resource Lambda" << std::endl;
    std::unique_lock<std::mutex> lock(foundDevicesMutex);
    auto resourceSid = resource->sid();
    std::cout << "Found resource " << resource->uniqueIdentifier() <<std::endl;
    auto resourceUri = resource->uri();
    std::cout << "\tURI of the resource: " << resourceUri << std::endl;

    if (foundDevices.find(resourceSid)==foundDevices.end()) {
      std::cout << "\tNew SID: " << resourceSid << std::endl;
      foundDevices[resourceSid].resources[resourceUri]=resource;
    } else {
      if (foundDevices[resourceSid].resources.find(resourceUri) == foundDevices[resourceSid].resources.end()) {
        std::cout << "\tExisting SID and new URI" << std::endl;
        foundDevices[resourceSid].resources[resourceUri]=resource;
      } else {
        std::cout << "\tExisting SID and existing URI" << std::endl;
        return;
      }
    }
    auto types = resource->getResourceTypes();
    std::cout << "\tList of resource types: " << std::endl;
    for(auto &resourceTypes : types ) {
      std::cout << "\t\t" << resourceTypes << std::endl;
    }

    if (resourceUri == "/oic/d")
      getOcfDeviceResource(resource, resourceSid);
  
    if (std::find(types.begin(), types.end(), "oic.r.switch.binary") != types.end() )  {
      setOcfResourceHostCoaps(resource);
      if (op==getBinarySwitch)
        getResBinarySwitch(resource, resourceSid);
      else
        postSwitchValue(resource, value);
    }


  };
  OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, foundResourceLambda);
  std::cout << "Called findResource" << std::endl;

}




