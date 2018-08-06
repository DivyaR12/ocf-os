// client.cpp
/* 
 *	SPDX-License-Identifier:	BSD-3-Clause 
 * The above license addresses copyright obligations. Please refer to Section 10.4 of the 
 * ATIS Operating Procedures for information about patent licensing obligations under 
 * ATIS' Intellectual Property Rights Policy.
 */

#define __WITH_DTLS__

#include "../ocflightclient.hpp"


static FILE* client_open(const char* path, const char* mode) {
  std::cout <<"client_open. Path: "<< path << std::endl;
  if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME)) {
    auto res = fopen("./switch.dat", mode);
    if (res!=NULL) 
      std::cout << "Opened file ./switch.dat" << std::endl;
    else
      std::cout << "Failed to open file ./client.dat" << std::endl;
    return res;
  } else {
     return fopen(path, mode);
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

  if (argc !=1 ) {
    std::cout << "Usage: ./ocfSwitch <0|1>" << std::endl;
    return 0;
  }

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

  struct sigaction sa;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);
  std::cout << "Press Ctrl-C to quit...." << std::endl;


  OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, &foundResource);
  std::cout << "Called findResource" << std::endl;


  do {
    usleep(2000000);
  } while (quit != 1);


  // Perform platform clean up.
  OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);
  std::cout<< "OC Stopped" << std::endl;

    
  return 0;
}
