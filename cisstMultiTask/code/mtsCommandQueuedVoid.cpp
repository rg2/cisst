/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*

  Author(s):  Ankur Kapoor, Peter Kazanzides, Anton Deguet
  Created on: 2005-05-02

  (C) Copyright 2005-2014 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


#include <cisstMultiTask/mtsCommandQueuedVoid.h>
#include <cisstMultiTask/mtsCallableVoidBase.h>
#include <cisstMultiTask/mtsManagerLocal.h>

mtsCommandQueuedVoid::mtsCommandQueuedVoid(void):
    BaseType(),
    MailBox(0)
{}


mtsCommandQueuedVoid::mtsCommandQueuedVoid(mtsCallableVoidBase * callable,
                                           const std::string & name,
                                           mtsMailBox * mailBox,
#if !CISST_HAS_SAFETY_PLUGINS
                                           size_t size):
    BaseType(callable, name),
#else
                                           size_t size,
                                           const std::string & componentName,
                                           const std::string & interfaceName):
    BaseType(callable, name, componentName, interfaceName),
#endif
    MailBox(mailBox),
    BlockingFlagQueue(size, MTS_NOT_BLOCKING),
    FinishedEventQueue()
{
    mtsCommandWriteBase *cmd = 0;
    FinishedEventQueue.SetSize(size, cmd);
}


mtsCommandQueuedVoid * mtsCommandQueuedVoid::Clone(mtsMailBox * mailBox, size_t size) const
{
#if !CISST_HAS_SAFETY_PLUGINS
    return new mtsCommandQueuedVoid(this->Callable, this->Name,
                                    mailBox, size);
#else
    return new mtsCommandQueuedVoid(this->Callable, this->Name,
                                    mailBox, size, this->ComponentName, this->InterfaceName);
#endif
}


mtsExecutionResult mtsCommandQueuedVoid::Execute(mtsBlockingType blocking)
{
    return Execute(blocking, 0);
}

mtsExecutionResult mtsCommandQueuedVoid::Execute(mtsBlockingType blocking,
                                                 mtsCommandWriteBase *finishedEventHandler)
{
    // check if this command is enabled
    if (!this->IsEnabled()) {
        return mtsExecutionResult::COMMAND_DISABLED;
    }
    // check if there is a mailbox (i.e. if the command is associated to an interface
    if (!MailBox) {
        CMN_LOG_RUN_ERROR << "Class mtsCommandQueuedVoid: Execute: no mailbox for \""
                          << this->Name << "\"" << std::endl;
        return mtsExecutionResult::COMMAND_HAS_NO_MAILBOX;
    }
    // check if all queues have some space
    if (BlockingFlagQueue.IsFull() || FinishedEventQueue.IsFull()|| MailBox->IsFull()) {
        /*
        CMN_LOG_RUN_WARNING << "Class mtsCommandQueuedVoid: Execute: Queue full for \""
                            << this->Name << "\" ["
                            << BlockingFlagQueue.IsFull() << "|"
                            << FinishedEventQueue.IsFull() << "|"
                            << MailBox->IsFull() << "]";
        */
        std::stringstream ss;
        ss << "Class mtsCommandQueuedVoid: Execute: Queue full for \""
           << this->Name << "\" ["
           << BlockingFlagQueue.IsFull() << "|"
           << FinishedEventQueue.IsFull() << "|"
           << MailBox->IsFull() << "]";
#if CISST_HAS_SAFETY_PLUGINS
        // onset event
        if (SF::State::NORMAL == 
            GetSafetyCoordinator->GetInterfaceState(this->ComponentName,
                                                    this->InterfaceName,
                                                    SF::GCM::PROVIDED_INTERFACE))
        {
            GetSafetyCoordinator->GenerateEvent(// event name
                                                "EVT_COMMAND_QUEUE_FULL",
                                                // statemachine type
                                                SF::State::STATEMACHINE_PROVIDED,
                                                // description
                                                ss.str(),
                                                this->ComponentName,
                                                this->InterfaceName);
        }
#endif
        CMN_LOG_RUN_WARNING << ss.str() << std::endl;
        return mtsExecutionResult::COMMAND_ARGUMENT_QUEUE_FULL;
    }
#if CISST_HAS_SAFETY_PLUGINS
    else {
        // offset event
        if (SF::State::ERROR == 
            GetSafetyCoordinator->GetInterfaceState(this->ComponentName,
                                                    this->InterfaceName,
                                                    SF::GCM::PROVIDED_INTERFACE))
        {
            std::stringstream ss;
            ss << "Class mtsCommandQueuedVoid: Execute: Queue is being dequeued\""
               << this->Name << "\" ["
               << BlockingFlagQueue.IsFull() << "|"
               << FinishedEventQueue.IsFull() << "|"
               << MailBox->IsFull() << "]";
            GetSafetyCoordinator->GenerateEvent("/EVT_COMMAND_QUEUE_FULL",
                                                SF::State::STATEMACHINE_PROVIDED,
                                                ss.str(),
                                                this->ComponentName,
                                                this->InterfaceName);
        }
    }
#endif
    // copy the blocking flag to the local storage.
    if (!BlockingFlagQueue.Put(blocking)) {
        CMN_LOG_RUN_ERROR << "Class mtsCommandQueuedVoid: Execute: BlockingFlagQueue.Put failed for \""
                          << this->Name << "\"" << std::endl;
        cmnThrow("mtsCommandQueuedVoid: Execute: BlockingFlagQueue.Put failed");
        return mtsExecutionResult::UNDEFINED;
    }
    // copy the finished event to the local storage.
    if (!FinishedEventQueue.Put(finishedEventHandler)) {
        CMN_LOG_RUN_ERROR << "Class mtsCommandQueuedVoid: Execute: FinishedEventQueue.Put failed for \""
                          << this->Name << "\"" << std::endl;
        BlockingFlagQueue.Get();   // Remove the blocking flag that was already queued
        cmnThrow("mtsCommandQueuedVoid: Execute: FinishedEventQueue.Put failed");
        return mtsExecutionResult::UNDEFINED;
    }
    // finally try to queue to mailbox
    if (!MailBox->Write(this)) {
        CMN_LOG_RUN_ERROR << "Class mtsCommandQueuedVoid: Execute: Mailbox.Write failed for \""
                          << this->Name << "\"" <<  std::endl;
        BlockingFlagQueue.Get();   // Remove the blocking flag that was already queued
        FinishedEventQueue.Get();   // Remove the blocking flag that was already queued
        cmnThrow("mtsCommandQueuedVoid: Execute: MailBox.Write failed");
        return mtsExecutionResult::UNDEFINED;
    }
    return mtsExecutionResult::COMMAND_QUEUED;
}


mtsBlockingType mtsCommandQueuedVoid::BlockingFlagGet(void)
{
    return *(this->BlockingFlagQueue.Get());
}


mtsCommandWriteBase *mtsCommandQueuedVoid::FinishedEventGet(void)
{
    return *(this->FinishedEventQueue.Get());
}


std::string mtsCommandQueuedVoid::GetMailBoxName(void) const
{
    return this->MailBox ? this->MailBox->GetName() : "null pointer!";
}


void mtsCommandQueuedVoid::ToStream(std::ostream & outputStream) const
{
    outputStream << "mtsCommandQueuedVoid: Mailbox \""
                 << this->GetMailBoxName()
                 << "\" for command(void) using " << *(this->Callable)
                 << " currently " << (this->IsEnabled() ? "enabled" : "disabled");
}
