/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$

  Author(s):  Ali Uneri
  Created on: 2009-11-06

  (C) Copyright 2009 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

/*!
  \file
  \brief Device wrapper for Claron Micron Tracker.
  \ingroup cisstDevices

  \bug(auneri1) Hard coded to only support 640x480 resolution cameras.
  \bug(auneri1) Automatic light coolness adjustment using CoolCard is not working.

  \warning(auneri1) HdrEnabledSet is disabled, since it's removed in the new API.

  \todo(auneri1) CalibratePivot method needs outlier filtering.
  \todo(auneri1) ComputeCameraModel method needs error analysis.
  \todo(auneri1) Consider deriving from mtsTaskContinuous using an oscillating sleep.
  \todo(auneri1) Mapping from markerName to markerHandle.
  \todo(auneri1) Refactor the method of obtaining marker projections for the controllerQDevice.
  \todo(auneri1) Check for mtMeasurementHazardCode using Xform3D_HazardCodeGet.
  \todo(auneri1) Refactor devNDISerial Qt widgets to a general "tracker" widget.
  \todo(auneri1) Fix/suppress _WIN32_WINNT macro redefinition warning.
*/

#ifndef _devMicronTracker_h
#define _devMicronTracker_h

#include <cisstVector/vctDynamicNArray.h>
#include <cisstVector/vctFixedSizeVectorTypes.h>
#include <cisstMultiTask/mtsTaskPeriodic.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>
#include <cisstStereoVision/svlBufferSample.h>
#include <cisstDevices/devExport.h>  // always include last

#include <MTC.h>


class CISST_EXPORT devMicronTracker : public mtsTaskPeriodic
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_ALLOW_DEFAULT);

 protected:
    class Tool
    {
     public:
        Tool(void);
        ~Tool(void) {};

        std::string Name;
        mtHandle Handle;
        std::string SerialNumber;
        mtsInterfaceProvided * Interface;
        prmPositionCartesianGet TooltipPosition;
        prmPositionCartesianGet MarkerPosition;
        mtsDoubleVec MarkerProjectionLeft;
        mtsDoubleVec MarkerProjectionRight;

        vct3 TooltipOffset;
    };

 public:
    devMicronTracker(const std::string & taskName, const double period);
    devMicronTracker(const mtsTaskPeriodicConstructorArg &arg);
    ~devMicronTracker(void) {};

    void Configure(const std::string & filename = "");
    void Startup(void);
    void Run(void);
    void Cleanup(void);

    size_t GetNumberOfTools(void) const {
        return Tools.size();
    }
    std::string GetToolName(const unsigned int index) const;
    svlBufferSample * GetImageBufferLeft(void) {
        return ImageBufferLeft;
    }
    svlBufferSample * GetImageBufferRight(void) {
        return ImageBufferRight;
    }

 protected:
    enum { LEFT_CAMERA, RIGHT_CAMERA, MIDDLE_CAMERA };

    Tool * CheckTool(const std::string & serialNumber);
    Tool * AddTool(const std::string & name, const std::string & serialNumber);

    vctFrm3 XfHandleToFrame(mtHandle & xfHandle);
    mtHandle FrameToXfHandle(vctFrm3 & frame);

    void InitComponent(void);  // called from constructor

    void ToggleCapturing(const mtsBool & toggle);
    void ToggleTracking(const mtsBool & toggle);
    void Track(void);
    void CalibratePivot(const mtsStdString & toolName);
    void ComputeCameraModel(const mtsStdString & pathRectificationLUT);

    static const unsigned int FrameWidth = 640;
    static const unsigned int FrameHeight = 480;
    static const unsigned int FrameSize = FrameWidth * FrameHeight;

    std::string CameraCalibrationDir;
    std::string MarkerTemplatesDir;

    typedef cmnNamedMap<Tool> ToolsType;
    ToolsType Tools;

    mtsBool IsCapturing;
    mtsBool IsTracking;

    mtHandle CurrentCamera;
    mtHandle IdentifyingCamera;
    mtHandle IdentifiedMarkers;
    mtHandle PoseXf;
    mtHandle Path;
    mtsDoubleVec MarkerProjectionLeft;

    mtsStateTable ImageTable;
    mtsUCharVec ImageLeft;
    mtsUCharVec ImageRight;
    svlSampleImageRGB * RGB;
    svlBufferSample * ImageBufferLeft;
    svlBufferSample * ImageBufferRight;
};

CMN_DECLARE_SERVICES_INSTANTIATION(devMicronTracker);

#endif  //_devMicronTracker_h
