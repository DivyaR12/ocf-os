
#include <signal.h>
#include <thread>
#include <functional>
#include <string>
#include <iostream>
#include <memory>
#include <exception> 
#include <mutex>
#include <map>


#include "ocstack.h"
#include "OCPlatform.h"
#include "OCApi.h"
#include "ocpayload.h"

using namespace OC;
namespace PH = std::placeholders;

typedef std::map <std::string, std::shared_ptr<OC::OCResource> > uriToResourceMapType;

struct ocDeviceType {
  std::string deviceName;
  uriToResourceMapType resources;
};

typedef std::map <std::string, ocDeviceType > sidToDeviceMapType;

sidToDeviceMapType foundDevices;
std::mutex foundDevicesMutex;




void onPostSwitch(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
  if (eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED || eCode == OC_STACK_RESOURCE_CHANGED)
    std::cout << "POST request was successful" << std::endl;
}

void postSwitchValue(std::shared_ptr<OC::OCResource> resource, bool value) {
  OCRepresentation rep;
  rep.setValue("value", value);
  resource->post(rep, QueryParamsMap(), &onPostSwitch);
}


void onGetSwitch(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
  std::cout << "On Get Switch!" << std::endl;
  if (eCode == OC_STACK_OK) {
    bool value;
    bool found;
    found = rep.getValue("value", value);
    if (found)
      std::cout<< "Light switch value: "<< value << std::endl;
    else
      std::cout<< "NOT found a value" << std::endl;
    
  } else
   std::cout << "Stack error in onGetSwitch!" << std::endl;
}


void onGet(const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
  std::cout << "On Get!" << std::endl;
  if (eCode == OC_STACK_OK) {
    std::string name;
    rep.getValue("n", name);
    std::cout << "\tDevice Name: " << name << std::endl;
    std::string piid;
    rep.getValue("piid", piid);
    std::cout << "\tPiid: " << piid << std::endl;
    std::string di;
    rep.getValue("di", di);
    std::cout << "\tdi: "<< di << std::endl;
    std::string host;
    host = rep.getHost();
    std::cout << "\tHost: " << host << std::endl;
    {
      std::unique_lock<std::mutex> lock(foundDevicesMutex);
      if (foundDevices.find(di)!=foundDevices.end()) {
        foundDevices[di].deviceName = name;
        std::cout << "\tStored name in foundDevices" << std::endl;
      }
      else
        std::cout << "\tError: found name for unknown SID: " << di << std::endl;
    }
  } else
   std::cout << "Stack error in onGet!" << std::endl;
}



void foundResource(std::shared_ptr<OC::OCResource> resource) {
  std::cout << "Found resource" << std::endl;
  std::unique_lock<std::mutex> lock(foundDevicesMutex);
  auto resourceSid = resource->sid();
  std::cout << "Found resource " << resource->uniqueIdentifier() <<
                " on server with ID: "<< resourceSid <<std::endl;
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
  if (resourceUri == "/oic/d") {

    auto onGetLambda = [resourceSid] ( const HeaderOptions& headerOptions, const OCRepresentation& rep, const int eCode) {
      std::cout << "On Get Lambda!" << std::endl;
      if (eCode == OC_STACK_OK) {
        std::string name;
        rep.getValue("n", name);
        {
          std::unique_lock<std::mutex> lock(foundDevicesMutex);
          if (foundDevices.find(resourceSid)!=foundDevices.end()) {
              foundDevices[resourceSid].deviceName = name;
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
  if (resourceUri == "/binaryswitch") {
    QueryParamsMap test;
    resource->get(test, &onGetSwitch);
    postSwitchValue(resource, 0);
  }   

  std::cout << "\tList of resource types: " << std::endl;
  for(auto &resourceTypes : resource->getResourceTypes()) {
    std::cout << "\t\t" << resourceTypes << std::endl;
  }
}


// global needs static, otherwise it can be compiled out and then Ctrl-C does not work
static int quit = 0;
// handler for the signal to stop the application
void handle_signal(int signal)
{
    OC_UNUSED(signal);
    quit = 1;
}


// main application
// starts the client 
int main()
{
    // Create persistent storage handlers
    OCPersistentStorage ps{fopen, fread, fwrite, fclose, unlink};
    // create the platform
    PlatformConfig cfg
    {
        ServiceType::InProc,
        ModeType::Client,
        &ps
    };
    OCPlatform::Configure(cfg);
    OC_VERIFY(OCPlatform::start() == OC_STACK_OK);
    std::cout << "OC Started" << std::endl;



    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    std::cout << "Press Ctrl-C to quit...." << std::endl;


    //    requestURI << OC_RSRVD_WELL_KNOWN_URI;// << "?rt=core.light";

    OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, &foundResource);
    std::cout << "Called findResource" << std::endl;

    do
    {
        usleep(2000000);
    }
    while (quit != 1);

    // Perform platform clean up.
    OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);
    std::cout<< "OC Stopped" << std::endl;
    

    return 0;
}
