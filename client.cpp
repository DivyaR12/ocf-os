// client.cpp
/* 
 *	SPDX-License-Identifier:	BSD-3-Clause 
 * The above license addresses copyright obligations. Please refer to Section 10.4 of the 
 * ATIS Operating Procedures for information about patent licensing obligations under 
 * ATIS' Intellectual Property Rights Policy.
 */

#define __WITH_DTLS__

#include "ocflightclient.hpp"
#include "onem2m.hxx"
#include "SimpleOpt.h"

using namespace ::onem2m;

OCPlatformInfo platformInfo;

void duplicateString(char ** targetString, const std::string & sourceString) {
  *targetString = new char[sourceString.length() + 1];
  strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

void setPlatformInfo() {
  duplicateString(&platformInfo.platformID, "FEDCBA98-7654-3210-0123-456789ABCDEF");
  duplicateString(&platformInfo.manufacturerName, "OS-IoT Demo");
  duplicateString(&platformInfo.manufacturerUrl, "https://www.os-iot.org/");
  duplicateString(&platformInfo.modelNumber, "modelNumber");
  duplicateString(&platformInfo.dateOfManufacture, "2018-08-01");
  duplicateString(&platformInfo.platformVersion, "0.1");
  duplicateString(&platformInfo.operatingSystemVersion, "myOS");
  duplicateString(&platformInfo.hardwareVersion, "0.1");
  duplicateString(&platformInfo.firmwareVersion, "0.1");
  duplicateString(&platformInfo.supportUrl,  "https://www.os-iot.org/");
  duplicateString(&platformInfo.systemTime, "2018-08-01T12:00:00.52Z");
}

OCStackResult setDeviceInfo() {
  std::string  deviceName = "OS-IoT_Bridge";
  std::string  deviceType = "oic.d";
  std::string  specVersion = "ocf.1.0.0";
  std::vector<std::string> dataModelVersions;
  dataModelVersions.push_back("ocf.res.1.3.0");
  dataModelVersions.push_back("ocf.sh.1.3.0");
  std::string  protocolIndependentID = "ffffffff-aaaa-9999-8888-777777777777";
  OCStackResult result = OC_STACK_ERROR;

  OCResourceHandle handle = OCGetResourceHandleAtUri(OC_RSRVD_DEVICE_URI);
  if (handle == NULL) {
     std::cout << "Failed to find resource " << OC_RSRVD_DEVICE_URI << std::endl;
     return result;
  }
//  result = OCBindResourceTypeToResource(handle, deviceType.c_str());
//  if (result != OC_STACK_OK) {
//     std::cout << "Failed to add device type" << std::endl;
//     return result;
//  }
  result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DEVICE_NAME, deviceName);
  if (result != OC_STACK_OK) {
    std::cout << "Failed to set device name" << std::endl;
    return result;
  }
  result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_DATA_MODEL_VERSION,
                                          dataModelVersions);
  if (result != OC_STACK_OK) {
     std::cout << "Failed to set data model versions" << std::endl;
     return result;
  }
  result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_SPEC_VERSION, specVersion);
  if (result != OC_STACK_OK) {
     std::cout << "Failed to set spec version" << std::endl;
     return result;
  }
  result = OCPlatform::setPropertyValue(PAYLOAD_TYPE_DEVICE, OC_RSRVD_PROTOCOL_INDEPENDENT_ID,
                                          protocolIndependentID);
  if (result != OC_STACK_OK) {
     std::cout << "Failed to set piid" << std::endl;
     return result;
  }

  return OC_STACK_OK;
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

long createContentInstance(const std::string& address, const std::string value) {
  long result;
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;
  
  auto ci = ::onem2m::contentInstance();
  ci.contentInfo("application/text");
  ci.content (value);
  respObj = ::onem2m::createResource(address, "1234", ci, result, respObjType); 
  std::cout << "Create content instance result:" << result << "\n";
  return result;
} 

long createSubscription (const ::std::string& objectAddress, const std::string& notifUri, std::string& subsRi) {
  long result;
  
  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;
  auto sub = ::onem2m::subscription();
  auto uris = ::onem2m::listOfURIs();
  auto criteria = ::onem2m::eventNotificationCriteria();
  // If the complexType is declared in-line in the XSD file then the CodeSynthesis compiler generates an
  // automatically named type within the parent class - see example below
  auto events = ::onem2m::eventNotificationCriteria::notificationEventType_sequence();

  sub.resourceName("subscription");
  events.push_back( ::onem2m::updateOfResource ); // Add one notification case to the list of notification events
  criteria.notificationEventType(events); // Assign the list of events to the criteria
  sub.eventNotificationCriteria(criteria); // Assign the criteria to the subscription
  sub.notificationContentType(nctAllAttributes);
  uris.push_back(notifUri); // Add one notification URI
  sub.notificationURI(uris);
  sub.latestNotify(true);
  respObj = ::onem2m::createResource(objectAddress, "1234", sub, result, respObjType);
  if (respObjType == resourceTypeSubscription) {
    auto sPtr = static_cast< subscription* >(respObj.get()); 
    if (sPtr->resourceID().present ())
      subsRi= sPtr->resourceID() . get();
  }
  std::cout << "Create subscription result:" << result << "\n";
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

void updateCseContainers (const std::string & aeAddr, const std::string & poa) {
  std::unique_lock<std::mutex> lock(foundDevicesMutex);
  long result;
  for (auto & x : foundDevices) {
    if (x.second.isLight && (! x.second.isInCse) && (! x.second.deviceName.empty())) {
      std::cout << "New light: "<<x.second.deviceName<<std::endl;
      result = createContainer(aeAddr, x.second.deviceName);
      if (result == onem2mHttpCREATED) {
        x.second.isInCse = true;
        std::cout<<"Container created, name: "<<x.second.deviceName<<std::endl;
        if (x.second.gotValue) {
          result = createContentInstance(aeAddr+"/"+x.second.deviceName,x.second.value?"1":"0");
          if (result == onem2mHttpCREATED) 
            std::cout << "CI Created" << std::endl;
          else
            std::cout <<"Error creating CI: "<< result << std::endl;
        }
        std::string subResId = "";
        result = createSubscription( aeAddr+"/"+x.second.deviceName, poa, subResId);
        if (result == onem2mHttpCREATED && !subResId.empty()) {
          std::cout << "Subscription Created. ID: " << subResId << std::endl;
          if (!subResId.empty())
            subRiToSidMap[subResId] = x.first;
        } else
          std::cout <<"Error creating Subscription: "<< result << std::endl;
      } else
        std::cout<<"Error creating container: "<< result <<std::endl;
    }
  }
}

onem2mResponseStatusCode processNotification(std::string host, std::string& from, notification* notif ) {

  if (notif && notif->verificationRequest().present () && notif->verificationRequest().get()) { // Check if this is a vefification request
    // Whether to accept the verificationRequest could be decided here, e.g. by checking the "host" and "from" parameters.
    // In this implmentation we accept all verification requests.
    
    std::cout << "Notification verification from:" << from << std::endl ;
    from = getFrom();
    return rcOK;
  }
  if ( notif ) {
    std::cout << "Notification" << std::endl;
    if (notif->subscriptionReference().present()) {
      std::unique_lock<std::mutex> lock(foundDevicesMutex);
      auto thisSidPair = subRiToSidMap.find(notif->subscriptionReference().get());
      if(thisSidPair != subRiToSidMap.end()) {
        std::cout << "\tSID: " << thisSidPair->second << std::endl;
        auto  thisDevice = foundDevices.find(thisSidPair -> second);
        if (thisDevice != foundDevices.end() && thisDevice->second.isLight && 
            thisDevice->second.resources.find("/binaryswitch") != thisDevice->second.resources.end()) {
          std::cout << "\tFound OCF light record" <<std::endl;
          ::xml_schema::integer rot;
          ::xml_schema::type* resObjPtr;
          resObjPtr = notif->getRepresentationObject(rot);
          if (rot == resourceTypeContentInstance) {
             auto ciPtr = static_cast< contentInstance* >(resObjPtr);
             if (ciPtr->content().present()) {
               bool newValue = ciPtr->content().get()=="1";
               std::cout << "\tNew value: "<< newValue << std::endl;
               postSwitchValue(thisDevice->second.resources.find("/binaryswitch")->second, 
                               newValue);
               thisDevice->second.value = newValue;
             }
          }
        }
      } else 
        std::cout << "\tSubscription ID not found." << std::endl;
    }
  }
  return rcOK;

}

long startServer (const std::string & poaPort) {
  // Start an HTTP server
  long result;
  result = startHttpServer(std::vector< std::string >(),::std::stoi( poaPort ), &processNotification);
  if (result == long(onem2mHttpOK))
    std::cout << "HTTP Server started OK\n";
  else
    std::cout << "HTTP Server didn't start. Check port is not already in use. Result =" << result << "\n";
  return result;
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

static FILE* client_open(const char* path, const char* mode) {
  std::cout <<"client_open. Path: "<< path << std::endl;
  if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME)) {
    auto res = fopen("./client.dat", mode);
    if (res!=NULL) 
      std::cout << "Opened file ./client.dat" << std::endl;
    else
      std::cout << "Failed to open file ./client.dat" << std::endl;
    return res;
  } else {
     return fopen(path, mode);
  }
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
  ::std::string poaPort = "18888";
  ::std::string poaAddr = "127.0.0.1";
  bool deleteAe = false;
  bool testOnly = false;

  setPlatformInfo();
  OCPersistentStorage ps{client_open, fread, fwrite, fclose, unlink};
  PlatformConfig cfg {
    ServiceType::InProc,
    OC::ModeType::Both, // Have to set this if using security, even if we are a pure client, or else 
                        // iotivity won't read the security .dat file
    &ps
  };
  OCPlatform::Configure(cfg);
  OC_VERIFY(OCPlatform::start() == OC_STACK_OK);
  std::cout << "OC Started" << std::endl;

  OC_VERIFY(OCPlatform::registerPlatformInfo(platformInfo) == OC_STACK_OK);
  std::cout << "OC Platform Info Registered" << std::endl;

  OC_VERIFY(setDeviceInfo() == OC_STACK_OK);
  std::cout << "OC Device Info Registered" << std::endl;

  struct sigaction sa;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);
  std::cout << "Press Ctrl-C to quit...." << std::endl;

  enum { optHostName, optAddress, optFrom, optPskIdentity, optPskKey, optPskKeyAscii, optDeleteAe,
         optPoaPort, optPoaAddr, optTestOnly };
  CSimpleOpt::SOption  cmdOptions[] = {
  { optPskIdentity, "--pskIdentity", SO_REQ_SEP},
  { optPskKey, "--pskKey", SO_REQ_SEP},
  { optPskKeyAscii, "--pskKeyAscii", SO_REQ_SEP},
  { optDeleteAe, "--deleteAe", SO_NONE },
  { optTestOnly, "--testOnly", SO_NONE },
  { optHostName, "-h", SO_REQ_SEP },
  { optAddress, "-a", SO_REQ_SEP },
  { optPoaPort, "--poaPort", SO_REQ_SEP },
  { optPoaAddr, "--poaAddr", SO_REQ_SEP },
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
        case optPoaPort:
           poaPort = args.OptionArg();
           break;
        case optPoaAddr:
           poaAddr = args.OptionArg();
           break;
        case optDeleteAe:
           deleteAe=true;
           break;
        case optTestOnly:
           testOnly=true;
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
  if (! testOnly)
    startServer(poaPort);   

  ::xml_schema::integer respObjType;
  std::unique_ptr< ::xml_schema::type > respObj;
  long result;

  if (deleteAe) {
    std::cout << "Deleting AE \r\n";
    deleteResources(cseRootAddr +"/"+aeResourceName);
    ::onem2m::terminate();
    return 0;
  }
 
  if (!testOnly) {
    bridgeAeId=fromField;
    result = createAE(cseRootAddr, "ocfBridgeDemo", "", bridgeAeId, aeResourceName);
    if (result != onem2mHttpCREATED) {
      std::cout << "Unable to create AE resource. Problem with CSE? Exiting bridge app.\r\n";
      return 1;
    }
  }


  discoverOcfResources(getBinarySwitch, false);

  do {
    usleep(2000000);
    if (!testOnly)
      updateCseContainers (cseRootAddr+"/"+aeResourceName, "http://"+poaAddr+":"+poaPort+"/");
  } while (quit != 1);

  // Perform platform clean up.
  OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);
  std::cout<< "OC Stopped" << std::endl;

  if (!aeResourceName.empty() && !testOnly) {
    std::cout << "Deleting AE \r\n";
    deleteResources( cseRootAddr +"/"+aeResourceName); 
  }
 
  ::onem2m::terminate();
    
  return 0;
}
