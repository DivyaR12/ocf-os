// client.cpp
/* 
 *	SPDX-License-Identifier:	BSD-3-Clause 
 * The above license addresses copyright obligations. Please refer to Section 10.4 of the 
 * ATIS Operating Procedures for information about patent licensing obligations under 
 * ATIS' Intellectual Property Rights Policy.
 */

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
#include "onem2m.hxx"
#include "SimpleOpt.h"

using namespace OC;
using namespace ::onem2m;

namespace PH = std::placeholders;

typedef std::map <std::string, std::shared_ptr<OC::OCResource> > uriToResourceMapType;

struct ocDeviceType {
  std::string deviceName = "";
  bool isLight = false;
  bool isInCse = false;
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
  if (resourceUri == "/oic/d")
    getOcfDeviceResource(resource, resourceSid);

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




std::unique_ptr< ::xml_schema::type >  retrieveResources (const ::std::string & cse_root_addr, long & result,  		::xml_schema::integer & respObjType) {
  std::unique_ptr< ::xml_schema::type > respObj;
  

   // *** Retrieve a resource ***
  respObj = retrieveResource(cse_root_addr, "1234", result, respObjType);
  std::cout << "Retrieve result:" << result << "\n";
  std::cout << "Obj Type#:" << respObjType << "\n";
  // Example of how to check the type of an object returned by the send operation and extract its contents
  if (respObjType == resourceTypeCSEBase) {
    CSEBase* csePtr = static_cast< CSEBase* >(respObj.get());
    if (csePtr->supportedResourceType().present ())
      std::cout << "Supported resource types:" << csePtr-> supportedResourceType () . get() << "\n";
    if (csePtr->accessControlPolicyIDs().present ()) {
      std::cout << "Access Control Policy ID(s):";
      for (uint i=0; i< csePtr->accessControlPolicyIDs().get().size(); i++)
        std::cout << csePtr->accessControlPolicyIDs().get()[i] << " ";
      std::cout << "\n";
    } 
  } 
  if (respObjType == resourceTypeContentInstance) {
    contentInstance* ciPtr = static_cast< contentInstance* >(respObj.get());
    if (ciPtr->content().present ())
      std::cout << "Content returned:" << ciPtr-> content () . get() << "\n";
  }

  return respObj;
}

long createAE(const ::std::string & cseRootAddr, const ::std::string & appId, const ::std::string & labelText, ::std::string & aeId, ::std::string & resourceName) {
  long result;
  
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;

  auto ae = AE();
  if (!resourceName.empty())
    ae.resourceName(resourceName);
  ae.App_ID(appId);
  ae.requestReachability(false); // The CSE can't send us a requestPrimitive directly (because we don't have an HTTP server to receive it)
  if (labelText.length()>0) {
    auto lb = labels ();
    lb.push_back(labelText);
    ae.labels(lb);
  }

  respObj = createResource(cseRootAddr, "1234", ae, result, respObjType);  
  std::cout << "Create AE result:" << result << "\n";
  // Example of how to check the type of an object returned by the send operation and extract its contents
  if (respObjType == resourceTypeAE) {
    auto aePtr = static_cast< AE* >(respObj.get()); // Cast generic object to pointer to the appropriate specific child class
    if (aePtr->AE_ID().present ()) {
      std::cout << "AE-ID:" << aePtr->AE_ID() . get() << "\n";
      aeId= aePtr->AE_ID() . get();
    }
    if (aePtr->resourceName().present()) {
      std::cout << "New AE Resource Name:" << aePtr->resourceName() . get() << "\n";
      resourceName= aePtr->resourceName() . get();
    }
    
  }

  return result;
}

long createContainer (const ::std::string & addr, const ::std::string & name) {
  long result;
  
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;

  auto c = container();
  c.resourceName(name);
  c.maxNrOfInstances( 1 );
  respObj = createResource(addr, "1234", c, result, respObjType);  
  std::cout << "Create container result:" << result << "\n";

  return result;
}

long deleteResources (const ::std::string & addr) {
  long result;
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;

  // ** Delete the AE **
  respObj = deleteResource(addr, "9876", result, respObjType); 
  std::cout << "Delete result:" << result << "\n";
  return result;
}


void updateCseContainers (const std::string & aeAddr) {
  std::unique_lock<std::mutex> lock(foundDevicesMutex);
  long result;
  for (auto & x : foundDevices) {
    if (x.second.isLight && (! x.second.isInCse) && (! x.second.deviceName.empty())) {
      std::cout << "New light: "<<x.second.deviceName<<std::endl;
      result = createContainer(aeAddr, x.second.deviceName);
      if (result == onem2mHttpCREATED) {
        x.second.isInCse = true;
        std::cout<<"Container created, name: "<<x.second.deviceName<<std::endl;
      } else
        std::cout<<"Error creating container: "<< result <<std::endl;
    }
  }
}


std::string stringToHexString(const std::string& input) {
  static const char* const lut = "0123456789abcdef";
  size_t len = input.length();

  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}

const char * getLastErrorText( int a_nError ) {
    switch (a_nError) {
    case SO_SUCCESS:            return "Success";
    case SO_OPT_INVALID:        return "Unrecognized option";
    case SO_OPT_MULTIPLE:       return "Option matched multiple strings";
    case SO_ARG_INVALID:        return "Option does not accept argument";
    case SO_ARG_INVALID_TYPE:   return "Invalid argument format";
    case SO_ARG_MISSING:        return "Required argument is missing";
    case SO_ARG_INVALID_DATA:   return "Invalid argument data";
    default:                    return "Unknown error";
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
int main (int argc, char* argv[]) {
  ::onem2m::initialize();
  // setDebug( false );

  ::onem2m::setHostName("127.0.0.1:8080");
  ::std::string cseRootAddr = "/in-cse/in-name"; 
  ::std::string bridgeAeId;
  ::std::string fromField = "admin:admin";
  ::std::string aeResourceName = "OCFBridge";
  bool deleteAe = false;

  OCPersistentStorage ps{fopen, fread, fwrite, fclose, unlink};
  PlatformConfig cfg {
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

  enum { optHostName, optAddress, optFrom, optPskIdentity, optPskKey, optPskKeyAscii, optDeleteAe };
  CSimpleOpt::SOption  cmdOptions[] = {
  { optPskIdentity, "--pskIdentity", SO_REQ_SEP},
  { optPskKey, "--pskKey", SO_REQ_SEP},
  { optPskKeyAscii, "--pskKeyAscii", SO_REQ_SEP},
  { optDeleteAe, "--deleteAe", SO_NONE },
  { optHostName, "-h", SO_REQ_SEP },
  { optAddress, "-a", SO_REQ_SEP },
  { optFrom, "-f", SO_REQ_SEP },
  SO_END_OF_OPTIONS 
  };

  CSimpleOpt args(argc, argv, cmdOptions, SO_O_NOSLASH | SO_O_SHORTARG | SO_O_CLUMP);
  while(args.Next()) {
    if (args.LastError() == SO_SUCCESS) {
      switch (args.OptionId()) {
        case optHostName:
           setHostName( args.OptionArg() );
           break;
        case optAddress:
          cseRootAddr=std::string( args.OptionArg() );
          break;
        case optFrom:
          fromField = args.OptionArg();
          break;
        case optPskIdentity:
           setTransport(transportHttpsPsk);
           setPskIdentity( args.OptionArg() );
           break;
        case optPskKey:
           setPskKey( args.OptionArg() );
           break;
        case optPskKeyAscii:
           setPskKey( stringToHexString( args.OptionArg() ) );
           break;
        case optDeleteAe:
           deleteAe=true;
           break;
        default:
          std::cout << "Unexpected option" << std::endl;
          break;
      }
    } else { // Error in arguments
      std::cout << "Error in arguments.\r\n";
      std::cout << "Option: " << args.OptionText() << " Error: " << getLastErrorText(args.LastError()) << "\r\n";
      std::cout << "Data: " << args.OptionArg() <<"\r\n";
      return 1;
    }
  }


  setFrom(fromField);
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;
  long result;

  if (deleteAe) {
    std::cout << "Deleting AE \r\n";
    deleteResources(fromField);
    ::onem2m::terminate();
    return 0;
  }

  bridgeAeId=fromField;
  result = createAE(cseRootAddr, "ocfBridgeDemo", "", bridgeAeId, aeResourceName);
  if (result != onem2mHttpCREATED) {
    std::cout << "Unable to create AE resource. Problem with CSE? Exiting bridge app.\r\n";
    return 1;
  }


  OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, &foundResource);
  std::cout << "Called findResource" << std::endl;

  do {
    usleep(2000000);
    updateCseContainers (cseRootAddr+"/"+aeResourceName);
  } while (quit != 1);

  // Perform platform clean up.
  OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);
  std::cout<< "OC Stopped" << std::endl;

  if (!aeResourceName.empty()) {
    std::cout << "Deleting AE \r\n";
    deleteResources( cseRootAddr +"/"+aeResourceName); 
  }
 
  ::onem2m::terminate();
    
  return 0;
}
