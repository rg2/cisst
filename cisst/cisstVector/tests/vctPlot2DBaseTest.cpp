/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  $Id$

  Author(s):  Joshua Chuang
  Created on: 2011-06-01

  (C) Copyright 2011 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include "vctPlot2DBaseTest.h"

void vctPlot2DBaseTest::TestBufferManipulating(void)
{
    vctPlot2DBaseTestClass plot;
    vctPlot2DBaseTestClass::Signal * signal = plot.AddSignal("TestSignal");
    const size_t dataElements = 1000;
    const size_t pointSize = 2;
    double * arrayToAppend = new double[dataElements * pointSize];
    double * arrayToPrepend = new double[dataElements * pointSize];
    double * defaultArray = new double[dataElements * pointSize];
    size_t numberOfPoints, bufferSize;

    size_t i, j;

    // 100~199, append array
    j = 0;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        arrayToAppend[i] = dataElements + j;
        arrayToAppend[i + 1] = dataElements + j;
        j++;
    }

    // 0 ~ 99, default array
    j = 0;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        defaultArray[i] = j;
        defaultArray[i + 1] = j;
        j++;
    }

    // -100 ~ -1, prepend array
    j = 0-dataElements;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        arrayToPrepend[i] = j;
        arrayToPrepend[i + 1] = j;
        j++;
    }

    /****** TEST AppendPoint() ******/
    signal->SetSize(dataElements);
    // 0 ~ 99
    for (double i = 0; i < dataElements; i++) {
        signal->AppendPoint(vctDouble2(i, i));
    }

    vctDouble2 point;
    for (i = 0; i < dataElements; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == i);
        CPPUNIT_ASSERT(point.Y() == i);
    }

    /****** TEST Resize(), Expand ******/
    signal->Resize(dataElements * 2);
    bufferSize = signal->GetSize();
    numberOfPoints = signal->GetNumberOfPoints();
    CPPUNIT_ASSERT(numberOfPoints == dataElements);
    CPPUNIT_ASSERT(bufferSize == dataElements * 2);
    // 0 ~ 99, After expanding
    for (i = 0; i < dataElements; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == i);
        CPPUNIT_ASSERT(point.Y() == i);
    }

    /****** TEST AppendArray() ******/
    CPPUNIT_ASSERT(signal->AppendArray(arrayToAppend, dataElements * pointSize));

    // 0~199, After append
    for (i = 0; i < dataElements * 2; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == i);
        CPPUNIT_ASSERT(point.Y() == i);
    }

    /****** TEST Resize(), Shrink, trimOlder = false  ******/
    signal->Resize(dataElements, false);
    bufferSize = signal->GetSize();
    numberOfPoints = signal->GetNumberOfPoints();
    CPPUNIT_ASSERT(numberOfPoints == dataElements);
    CPPUNIT_ASSERT(bufferSize == dataElements);
    //  0~99, After shrinking
    for (i = 0; i < dataElements; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == i);
        CPPUNIT_ASSERT(point.Y() == i);
    }

    signal->AppendArray(defaultArray, dataElements * pointSize);
    signal->Resize(dataElements * 2);
    signal->PrependArray(arrayToPrepend, dataElements * pointSize);
    // -100 ~ 99
    for (i = 0; i < dataElements * 2; i++){
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i - dataElements));
        CPPUNIT_ASSERT(point.Y() == (i - dataElements));
    }

    // Set array to 0 ~ 199
    signal->Resize(dataElements * 2);
    CPPUNIT_ASSERT(signal->AppendArray(arrayToAppend, dataElements * pointSize));


    /****** TEST Resize(), Shrink, trimOlder = true ******/
    signal->Resize(dataElements);
    bufferSize = signal->GetSize();
    numberOfPoints = signal->GetNumberOfPoints();
    CPPUNIT_ASSERT(numberOfPoints == dataElements);
    CPPUNIT_ASSERT(bufferSize == dataElements);

    // 100~199, After shrinking
    for (i = 0; i < dataElements; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == i + dataElements);
        CPPUNIT_ASSERT(point.Y() == i + dataElements);
    }

    /****** TEST Prepend() ******/
    signal->Resize(dataElements * 2);
    // do twice, see if the data is replaced correctly
    signal->PrependArray(arrayToAppend, dataElements * pointSize);
    signal->PrependArray(arrayToPrepend, dataElements * pointSize);
    bufferSize = signal->GetSize();
    numberOfPoints = signal->GetNumberOfPoints();
    CPPUNIT_ASSERT(numberOfPoints == dataElements * 2);
    CPPUNIT_ASSERT(bufferSize == dataElements * 2);
    for (i = dataElements; i < dataElements * 2 ; i++) {
        signal->SetPointAt(i, vctDouble2(i - dataElements, i - dataElements));
    }
    // -100 ~ 99
    for (i = 0; i < dataElements * 2; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i - dataElements));
        CPPUNIT_ASSERT(point.Y() == (i - dataElements));
    }
    // test by overflowing  buffer
    // -99 ~ 100
    signal->AppendPoint(vctDouble2(dataElements, dataElements));
    signal->PrependArray(arrayToPrepend, dataElements * pointSize);
    // -100~-1, -99 ~ 0
    for (i = 0; i < dataElements; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i - dataElements));
        CPPUNIT_ASSERT(point.Y() == (i - dataElements));
    }
    for (i = dataElements; i < dataElements * 2; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i % dataElements - dataElements + 1));
        CPPUNIT_ASSERT(point.Y() == (i % dataElements - dataElements + 1));
    }

    /****** TEST SetArrayAt() ******/
    signal->SetArrayAt(0, arrayToPrepend, dataElements * pointSize);
    signal->SetArrayAt(dataElements, arrayToPrepend, dataElements * pointSize);

    // -100 ~ -1, -100~-1
    for (i = 0; i < dataElements * 2; i++) {
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i % dataElements - dataElements));
        CPPUNIT_ASSERT(point.Y() == (i % dataElements - dataElements));
    }

    /****** TEST SetPointAt() ******/
    // -100 ~ -1, -100~-1
    for (i = 0; i < dataElements * 2; i++) {
        point = signal->GetPointAt(i);
        signal->SetPointAt(i, point);
        point = signal->GetPointAt(i);
        CPPUNIT_ASSERT(point.X() == (i % dataElements - dataElements));
        CPPUNIT_ASSERT(point.Y() == (i % dataElements - dataElements));
    }
    delete arrayToAppend;
    delete arrayToPrepend;
}


void vctPlot2DBaseTest::TestRangeComputation(void){

    vctPlot2DBaseTestClass plot;
    vctPlot2DBaseTestClass::Signal * signal = plot.AddSignal("TestRange");
    const int dataElements = 12345;
    const int pointSize = 2;
    double * arrayToAppend = new double[dataElements * pointSize];
    double * arrayToPrepend = new double[dataElements * pointSize];
    double * defaultArray = new double[dataElements * pointSize];

    size_t i, j;

    // 100~199, append array
    j = 0;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        arrayToAppend[i] = dataElements + j;
        arrayToAppend[i+1] = dataElements + j;
        j++;
    }

    // 0 ~ 99, default array
    j = 0;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        defaultArray[i] = j;
        defaultArray[i + 1] = j;
        j++;
    }

    // -100 ~ -1, prepend array
    j = 0 - dataElements;
    for (i = 0; i < dataElements * pointSize; i += pointSize) {
        arrayToPrepend[i] = j;
        arrayToPrepend[i + 1] = j;
        j++;
    }

    signal->SetSize(dataElements);
    for (double i = 1; i <= dataElements; i++) {
        signal->AppendPoint(vctDouble2(i, i));
    }

    double  min, max;
    /****** TEST ComputeDataRangeX() , sorted = true******/
    signal->ComputeDataRangeX(min, max, true);
    CPPUNIT_ASSERT(min == 1.0) ;
    CPPUNIT_ASSERT(max == dataElements);

	min = max = 0;
    /****** TEST ComputeDataRangeX() , sorted = false******/
    signal->ComputeDataRangeX(min, max, false);
    CPPUNIT_ASSERT(min == 1.0) ;
    CPPUNIT_ASSERT(max == dataElements);

	min = max = 0;
    /****** TEST ComputeDataRangeY() ******/
    signal->ComputeDataRangeY(min, max);
    CPPUNIT_ASSERT(min == 1.0) ;
    CPPUNIT_ASSERT(max == dataElements);

	min = max = 0;
    /****** TEST ComputeDataRangeX(), by overflowing buffer ******/
    signal->AppendPoint(vctDouble2(0, 0));
    signal->ComputeDataRangeX(min, max, false);
    CPPUNIT_ASSERT(min == 0.0) ;
    CPPUNIT_ASSERT(max == dataElements);

	min = max = 0;
    /****** TEST ComputeDataRangeY(), by overflowing buffer ******/
    signal->ComputeDataRangeY(min, max);
    CPPUNIT_ASSERT(min == 0.0) ;
    CPPUNIT_ASSERT(max == dataElements);
    // I have to use static to declare
    vctDouble2 minXY, maxXY;
    signal->AppendPoint(vctDouble2(dataElements + 1, dataElements + 1));
    signal->ComputeDataRangeXY(minXY, maxXY);
    CPPUNIT_ASSERT(minXY.X() == 0.0);
    CPPUNIT_ASSERT(minXY.Y() == 0.0);
    CPPUNIT_ASSERT(maxXY.X() == dataElements + 1);
    CPPUNIT_ASSERT(maxXY.Y() == dataElements + 1);
}


CPPUNIT_TEST_SUITE_REGISTRATION(vctPlot2DBaseTest);
