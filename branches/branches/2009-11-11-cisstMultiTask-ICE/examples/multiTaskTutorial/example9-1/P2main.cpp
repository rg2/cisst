/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/* $Id: serverMain.cpp 1082 2010-01-25 21:56:21Z mjung5 $ */

#include <cisstCommon.h>
#include <cisstOSAbstraction.h>
#include <cisstMultiTask.h>

#include "C2ServerTask.h"
#include "C3Task.h"

int main(int argc, char * argv[])
{
    // TODO: uncomment this
    //if (argc != 2) {
    //    std::cerr << "Usage: " << argv[0] << "[global component manager IP]" << std::endl;
    //    exit(-1);
    //}
    
    // Set global component manager IP
    //const std::string globalComponentManagerIP(argv[1]);
    const std::string globalComponentManagerIP("127.0.0.1");
    std::cout << "Global component manager IP is set as " << globalComponentManagerIP << std::endl;

    // log configuration
    cmnLogger::SetLoD(CMN_LOG_LOD_VERY_VERBOSE);
    cmnLogger::GetMultiplexer()->AddChannel(std::cout, CMN_LOG_LOD_VERY_VERBOSE);
    // add a log per thread
    osaThreadedLogFile threadedLog("example9Server");
    cmnLogger::GetMultiplexer()->AddChannel(threadedLog, CMN_LOG_LOD_VERY_VERBOSE);

    // Get the local component manager
    mtsManagerLocal * localManager;
    try {
        localManager = mtsManagerLocal::GetInstance("P2", globalComponentManagerIP);
    } catch (...) {
        CMN_LOG_INIT_ERROR << "Failed to initialize local component manager" << std::endl;
        return 1;
    }

    // create our server task
    const double PeriodClient = 10 * cmn_ms; // in milliseconds
    C2ServerTask * C2Server = new C2ServerTask("C2", PeriodClient);
    C3Task * C3 = new C3Task("C3", PeriodClient);
    localManager->AddComponent(C2Server);
    localManager->AddComponent(C3);

    if (!localManager->Connect("C3", "r1", "C2", "p2")) {
        CMN_LOG_INIT_ERROR << "Connect failed: C3:r1-C2:p2" << std::endl;
        return 1;
    }
    
    // create the tasks, i.e. find the commands
    localManager->CreateAll();
    // start the periodic Run
    localManager->StartAll();
    
    while (1) {
        osaSleep(10 * cmn_ms);
    }
    
    // cleanup
    localManager->KillAll();
    localManager->Cleanup();

    return 0;
}

/*
  Author(s):  Min Yang Jung
  Created on: 2009-01-27

  (C) Copyright 2010 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/
