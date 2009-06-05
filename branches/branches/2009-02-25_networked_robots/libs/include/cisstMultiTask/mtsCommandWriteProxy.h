/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id: mtsCommandWriteProxy.h 75 2009-02-24 16:47:20Z adeguet1 $

  Author(s):  Min Yang Jung
  Created on: 2009-04-29

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/


/*!
  \file
  \brief Defines a command with one argument 
*/

#ifndef _mtsCommandWriteProxy_h
#define _mtsCommandWriteProxy_h

#include <cisstMultiTask/mtsCommandReadOrWriteBase.h>
#include <cisstCommon/cmnSerializer.h>

/*!
  \ingroup cisstMultiTask
*/
class mtsCommandWriteProxy: public mtsCommandWriteBase {
public:
    typedef mtsCommandWriteBase BaseType;

protected:
    std::stringstream StreamBufferInput;
    cmnSerializer * Serializer;
    mtsDeviceInterfaceProxyClient * ProvidedInterfaceProxy;

    /*! ID assigned by the server as a pointer to the actual command in server's
        memory space. */
    const int CommandSID;

    void Initialize() {
        if (!Serializer) {
            Serializer = new cmnSerializer(StreamBufferInput);
        }
    }

public:
    mtsCommandWriteProxy(const int commandSID, 
                        mtsDeviceInterfaceProxyClient * providedInterfaceProxy) 
        : CommandSID(commandSID), ProvidedInterfaceProxy(providedInterfaceProxy), BaseType()
    {
        Initialize();
    }

    mtsCommandWriteProxy(const int commandSID,
                         mtsDeviceInterfaceProxyClient * providedInterfaceProxy,
                         const std::string & name)
        : CommandSID(commandSID), ProvidedInterfaceProxy(providedInterfaceProxy), BaseType(name)
    {
        Initialize();
    }

    /*! The destructor. Does nothing */
    virtual ~mtsCommandWriteProxy() 
    {}

    /*! The execute method. */
    virtual mtsCommandBase::ReturnType Execute(const cmnGenericObject & argument) {
        Serializer->Serialize(argument);
        std::string streamData = StreamBufferInput.str();

        ProvidedInterfaceProxy->InvokeExecuteCommandWriteSerialized(CommandSID, streamData);

        return mtsCommandBase::DEV_OK;
    }

    /*! For debugging. Generate a human readable output for the
      command object */
    void ToStream(std::ostream & outputStream) const {
        outputStream << "mtsCommandWriteProxy: ";
        outputStream << "commandID is " << CommandSID << std::endl;
    }

    /*! Return a pointer on the argument prototype */
    const cmnGenericObject * GetArgumentPrototype(void) const {
        //
        // TODO: FIX THIS
        //
        return reinterpret_cast<const cmnGenericObject *>(0x1234);
    }
};

#endif // _mtsCommandWriteProxy_h
