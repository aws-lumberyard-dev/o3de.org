/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#ifndef CARRIER_DATAVIEW_H
#define CARRIER_DATAVIEW_H

#if !defined(Q_MOC_RUN)
#include <AzCore/base.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>

#include "Source/Driller/StripChart.hxx"
#include "Source/Driller/DrillerOperationTelemetryEvent.h"
#include "Source/Driller/DrillerDataTypes.h"
#endif

namespace Ui
{
    class CarrierDataView;
}

namespace Driller
{
    class CarrierDataAggregator;

    class CarrierDataView
        : public QDialog
    {
        Q_OBJECT
    public:
        AZ_CLASS_ALLOCATOR(CarrierDataView, AZ::SystemAllocator, 0);
        CarrierDataView(FrameNumberType startFrame, FrameNumberType endFrame, const CarrierDataAggregator* aggr);
        virtual ~CarrierDataView();

        // A data point is made up of a x and y value (first and second respectively) 
        // which can be plotted onto a XY chart.
        typedef AZStd::pair<AZ::s64, float> DataPoint;
        typedef AZStd::vector<DataPoint>    DataPointList;

    public slots:
        void OnCurrentFilterChanged();

    private:
        void SetupAllCharts(AZStd::string id, 
                            const CarrierDataAggregator* aggr,
                            FrameNumberType startFrame, 
                            FrameNumberType endFrame);

        void SetupDualBytesChart(StripChart::DataStrip* chart, const DataPointList& bytes0, const DataPointList& bytes1);
        void SetupDualPacketChart(StripChart::DataStrip* chart, const DataPointList& packets1, const DataPointList& packets2);
        void SetupTimeChart(StripChart::DataStrip* chart, const DataPointList& time);
        void SetupPercentageChart(StripChart::DataStrip* chart, const DataPointList& percentage);

        const CarrierDataAggregator* mAggregator;
        FrameNumberType mStartFrame, mEndFrame;
        
        DrillerWindowLifepsanTelemetry m_lifespanTelemetry;

        // A QT widget defined in CarrierDataView.ui and compiled into ui_CarrierDataView.hpp.
        Ui::CarrierDataView* m_gui; 
    };
}

#endif
