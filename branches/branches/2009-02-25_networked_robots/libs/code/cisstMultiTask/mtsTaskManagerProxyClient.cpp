/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsTaskManagerProxyClient.cpp 145 2009-03-18 23:32:40Z mjung5 $

  Author(s):  Min Yang Jung
  Created on: 2009-03-17

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <cisstMultiTask/mtsTaskManager.h>
#include <cisstMultiTask/mtsTaskManagerProxyClient.h>
#include <cisstOSAbstraction/osaSleep.h>

#define mtsTaskManagerProxyClientLogger(_log) \
    Logger->trace("mtsTaskManagerProxyClient", _log)

//#define _COMMUNICATION_TEST_

void mtsTaskManagerProxyClient::Start(mtsTaskManager * callingTaskManager)
{
    // Initialize Ice object.
    // Notice that a worker thread is not created right now.
    Init();
    
    if (InitSuccessFlag) {
        // Client configuration for bidirectional communication
        // (see http://www.zeroc.com/doc/Ice-3.3.1/manual/Connections.38.7.html)
        Ice::ObjectAdapterPtr adapter = IceCommunicator->createObjectAdapter("");
        Ice::Identity ident;
        ident.name = GetGUID();
        ident.category = "";    // not used currently.

        mtsTaskManagerProxy::TaskManagerClientPtr client = 
            new TaskManagerClientI(IceCommunicator, Logger, TaskManagerServer, this);
        adapter->add(client, ident);
        adapter->activate();
        TaskManagerServer->ice_getConnection()->setAdapter(adapter);

        // Set an implicit context (per proxy context)
        // (see http://www.zeroc.com/doc/Ice-3.3.1/manual/Adv_server.33.12.html)
        IceCommunicator->getImplicitContext()->put(CONNECTION_ID, 
            IceCommunicator->identityToString(ident));

        // Generate an event so that the global task manager register this task manager.
        TaskManagerServer->AddClient(ident);

        // Create a worker thread here and returns immediately.
        ThreadArgumentsInfo.argument = callingTaskManager;
        ThreadArgumentsInfo.proxy = this;        
        ThreadArgumentsInfo.Runner = mtsTaskManagerProxyClient::Runner;

        WorkerThread.Create<ProxyWorker<mtsTaskManager>, ThreadArguments<mtsTaskManager>*>(
            &ProxyWorkerInfo, &ProxyWorker<mtsTaskManager>::Run, &ThreadArgumentsInfo, "S-PRX");
    }
}

void mtsTaskManagerProxyClient::StartClient()
{
    Sender->Start();

    // This is a blocking call that should run in a different thread.
    IceCommunicator->waitForShutdown();
}

void mtsTaskManagerProxyClient::Runner(ThreadArguments<mtsTaskManager> * arguments)
{
    mtsTaskManager * TaskManager = reinterpret_cast<mtsTaskManager*>(arguments->argument);

    mtsTaskManagerProxyClient * ProxyClient = 
        dynamic_cast<mtsTaskManagerProxyClient*>(arguments->proxy);

    ProxyClient->GetLogger()->trace("mtsTaskManagerProxyClient", "Proxy client starts.");

    try {
        ProxyClient->StartClient();        
    } catch (const Ice::Exception& e) {
        ProxyClient->GetLogger()->trace("mtsTaskManagerProxyClient exception: ", e.what());
    } catch (const char * msg) {
        ProxyClient->GetLogger()->trace("mtsTaskManagerProxyClient exception: ", msg);
    }

    ProxyClient->OnThreadEnd();
}

void mtsTaskManagerProxyClient::OnThreadEnd()
{
    mtsTaskManagerProxyClientLogger("Proxy client ends.");

    BaseType::OnThreadEnd();

    Sender->Destroy();
}

//-------------------------------------------------------------------------
//  Send Methods
//-------------------------------------------------------------------------
bool mtsTaskManagerProxyClient::AddProvidedInterface(
    const std::string & newProvidedInterfaceName,
    const std::string & adapterName,
    const std::string & endpointInfo,
    const std::string & communicatorID,
    const std::string & taskName)
{
    ::mtsTaskManagerProxy::ProvidedInterfaceInfo info;
    info.adapterName = adapterName;
    info.endpointInfo = endpointInfo;
    info.communicatorID = communicatorID;
    info.taskName = taskName;
    info.interfaceName = newProvidedInterfaceName;

    GetLogger()->trace("TMClient", ">>>>> SEND: AddProvidedInterface: " 
        + info.taskName + ", " + info.interfaceName);

    return TaskManagerServer->AddProvidedInterface(info);
}

bool mtsTaskManagerProxyClient::AddRequiredInterface(
    const std::string & newRequiredInterfaceName, const std::string & taskName)
{
    ::mtsTaskManagerProxy::RequiredInterfaceInfo info;
    info.taskName = taskName;
    info.interfaceName = newRequiredInterfaceName;

    GetLogger()->trace("TMClient", ">>>>> SEND: AddRequiredInterface: " 
        + info.taskName + ", " + info.interfaceName);

    return TaskManagerServer->AddRequiredInterface(info);
}

bool mtsTaskManagerProxyClient::IsRegisteredProvidedInterface(
    const std::string & taskName, const std::string & providedInterfaceName) const
{
    GetLogger()->trace("TMClient", ">>>>> SEND: IsRegisteredProvidedInterface: " 
        + taskName + ", " + providedInterfaceName);

    return TaskManagerServer->IsRegisteredProvidedInterface(
        taskName, providedInterfaceName);
}

bool mtsTaskManagerProxyClient::GetProvidedInterfaceInfo(
    const ::std::string & taskName, const std::string & providedInterfaceName,
    ::mtsTaskManagerProxy::ProvidedInterfaceInfo & info) const
{
    GetLogger()->trace("TMClient", ">>>>> SEND: GetProvidedInterfaceInfo: " 
        + taskName + ", " + providedInterfaceName);

    return TaskManagerServer->GetProvidedInterfaceInfo(
        taskName, providedInterfaceName, info);
}

void mtsTaskManagerProxyClient::NotifyInterfaceConnectionResult(
    const bool isServerTask, const bool isSuccess,
    const std::string & userTaskName,     const std::string & requiredInterfaceName,
    const std::string & resourceTaskName, const std::string & providedInterfaceName)
{
    GetLogger()->trace("TMClient", ">>>>> SEND: NotifyInterfaceConnectionResult: " +
        resourceTaskName + " : " + providedInterfaceName + " - " +
        userTaskName + " : " + requiredInterfaceName);

    return TaskManagerServer->NotifyInterfaceConnectionResult(
        isServerTask, isSuccess, 
        userTaskName, requiredInterfaceName, resourceTaskName, providedInterfaceName);
}

//-------------------------------------------------------------------------
//  Definition by mtsTaskManagerProxy.ice
//-------------------------------------------------------------------------
mtsTaskManagerProxyClient::TaskManagerClientI::TaskManagerClientI(
    const Ice::CommunicatorPtr& communicator,                           
    const Ice::LoggerPtr& logger,
    const mtsTaskManagerProxy::TaskManagerServerPrx& server,
    mtsTaskManagerProxyClient * taskManagerClient)
    : Runnable(true), 
      Communicator(communicator), Logger(logger),
      Server(server), TaskManagerClient(taskManagerClient),      
      Sender(new SendThread<TaskManagerClientIPtr>(this))      
{
}

void mtsTaskManagerProxyClient::TaskManagerClientI::Start()
{
    mtsTaskManagerProxyClientLogger("Send thread starts");

    Sender->start();
}

void mtsTaskManagerProxyClient::TaskManagerClientI::Run()
{
    bool flag = true;

    while(true)
    {
#ifdef _COMMUNICATION_TEST_
        static int num = 0;
        std::cout << "client send: " << ++num << std::endl;
        Server->ReceiveDataFromClient(num);
#endif

        if (flag) {
            // Send a set of task names
            mtsTaskManagerProxy::TaskList localTaskList;
            std::vector<std::string> myTaskNames;
            mtsTaskManager::GetInstance()->GetNamesOfTasks(myTaskNames);

            localTaskList.taskNames.insert(
                localTaskList.taskNames.end(),
                myTaskNames.begin(),
                myTaskNames.end());

            localTaskList.taskManagerID = TaskManagerClient->GetGUID();

            Server->UpdateTaskManager(localTaskList);

            flag = false;
        }

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            timedWait(IceUtil::Time::seconds(2));
        }
    }
}

void mtsTaskManagerProxyClient::TaskManagerClientI::Destroy()
{
    mtsTaskManagerProxyClientLogger("Send thread is terminating.");

    IceUtil::ThreadPtr callbackSenderThread;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        mtsTaskManagerProxyClientLogger("Destroying sender.");
        Runnable = false;

        notify();

        callbackSenderThread = Sender;
        Sender = 0; // Resolve cyclic dependency.
    }

    callbackSenderThread->getThreadControl().join();
}

// for test purpose
void mtsTaskManagerProxyClient::TaskManagerClientI::ReceiveData(
    ::Ice::Int num, const ::Ice::Current&)
{
    std::cout << "------------ client recv data " << num << std::endl;
}


bool mtsTaskManagerProxyClient::TaskManagerClientI::ConnectAtServerSide(
    const std::string & userTaskName, const std::string & interfaceRequiredName,
    const std::string & resourceTaskName, const std::string & providedInterfaceName,
    const ::Ice::Current & current)
{
    //
    // 클라이언트에서 Connect() 성공했으므로
    // 서버에서도 req. interface proxy 만들어서 로컬 connect() 시도한다.
    //
    return true;
}