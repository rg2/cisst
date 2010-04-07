/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/* $Id$ */

#include <cisstCommon.h>
#include <cisstOSAbstraction.h>
#include <cisstMultiTask.h>

#include "serverTask.h"

int main(int argc, char * argv[])
{

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " GlobalManagerIP [flag]" << std::endl;
        std::cerr << "       flag = 1 to use double instead of mtsDouble" << std::endl;
        exit(-1);
    }

    CMN_LOG_INIT_WARNING << "Starting server, IP = " << argv[1] << std::endl;
    std::string globalComponentManagerIP(argv[1]);
    // Get the TaskManager instance and set operation mode
    mtsManagerLocal *taskManager = mtsManagerLocal::GetInstance(globalComponentManagerIP, "ProcessServer");

    // If flag is not specified, or not 1, then use generic type (mtsDouble)
    bool useGeneric = (argc > 2)?(argv[2][0] != '1'):true;

    // log configuration
    cmnLogger::SetLoD(CMN_LOG_LOD_VERY_VERBOSE);
    cmnLogger::GetMultiplexer()->AddChannel(std::cout, CMN_LOG_LOD_VERY_VERBOSE);
    // add a log per thread
    osaThreadedLogFile threadedLog("example9Server");
    cmnLogger::GetMultiplexer()->AddChannel(threadedLog, CMN_LOG_LOD_VERY_VERBOSE);
    // specify a higher, more verbose log level for these classes
    cmnClassRegister::SetLoD("mtsTaskInterface", CMN_LOG_LOD_VERY_VERBOSE);
    cmnClassRegister::SetLoD("mtsTaskManager", CMN_LOG_LOD_VERY_VERBOSE);
    cmnClassRegister::SetLoD("serverTask", CMN_LOG_LOD_VERY_VERBOSE);

    // create our server task
    const double PeriodServer = 10 * cmn_ms; // in milliseconds
    serverTaskBase *server;
    if (useGeneric)
        server = new serverTask<mtsDouble>("Server", PeriodServer);
    else
        server = new serverTask<double>("Server", PeriodServer);

    taskManager->AddTask(server);

    // create the tasks, i.e. find the commands
    taskManager->CreateAll();
    // start the periodic Run
    taskManager->StartAll();
    
    while (1) {
        osaSleep(10 * cmn_ms);
    }
    
    // cleanup
    taskManager->KillAll();
    taskManager->Cleanup();    
    return 0;
}

/*
  Author(s):  Ankur Kapoor, Peter Kazanzides, Anton Deguet, Min Yang Jung
  Created on: 2004-04-30

  (C) Copyright 2004-2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/
