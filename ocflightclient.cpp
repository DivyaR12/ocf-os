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

void getOcfSwitchResource(std::shared_ptr<OC::OCResource> resource, std::string resourceSid) {
  auto onGetSwitchLambda = [resourceSid] (const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
    std::cout << "On Get Switch Lambda!" << std::endl;
    std::unique_lock<std::mutex> lock(foundDevicesMutex);
    if (eCode == OC_STACK_OK && foundDevices.find(resourceSid)!=foundDevices.end()) {
      foundDevices[resourceSid].gotValue = rep.getValue("value", foundDevices[resourceSid].value);
      std::cout<<"\tSet SID:"<<resourceSid<<" to value "<<foundDevices[resourceSid].value<< std::endl;
    } else
     std::cout << "\tStack error in onGetSwitch or can't match SID!. ecode: " << eCode << std::endl;
  };
  std::cout << "getOcfSwitchResource. Host: " << resource->host() << std::endl;
  QueryParamsMap test;
  resource->get(test, onGetSwitchLambda);
}



void getOcfDeviceResource(std::shared_ptr<OC::OCResource> resource, std::string resourceSid) {
 auto onGetLambda = [resourceSid] ( const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
   std::cout << "On Get Lambda!" << std::endl;
    if (eCode == OC_STACK_OK) {
      std::string name;
      rep.getValue("n", name);
      std::replace( name.begin(), name.end(), ' ', '_'); // Remove whitespace in names
      auto types = rep.getResourceTypes();
      {
        std::unique_lock<std::mutex> lock(foundDevicesMutex);
        if (foundDevices.find(resourceSid)!=foundDevices.end()) {
          foundDevices[resourceSid].deviceName = name;
          foundDevices[resourceSid].isLight = std::find(types.begin(), types.end(), "oic.d.light") != types.end();
           std::cout << "\tStored name in foundDevices" << std::endl;
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
    if (resourceUri == "/oic/d")
      getOcfDeviceResource(resource, resourceSid);
  
    if (resourceUri == "/binaryswitch" || resourceUri == "/BinarySwitchResURI") {
      setOcfResourceHostCoaps(resource);
      if (op==getBinarySwitch)
        getOcfSwitchResource(resource, resourceSid);
      else
        postSwitchValue(resource, value);
    }

    std::cout << "\tList of resource types: " << std::endl;
    for(auto &resourceTypes : resource->getResourceTypes()) {
      std::cout << "\t\t" << resourceTypes << std::endl;
    }
  };
  OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, foundResourceLambda);
  std::cout << "Called findResource" << std::endl;

}




