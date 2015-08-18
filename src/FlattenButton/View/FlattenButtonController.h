// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#pragma once

#include "Types.h"
#include "BidirectionalBus.h"
#include "IFlattenButtonView.h"
#include "IFlattenButtonViewModel.h"
#include "ApplyScreenControl.h"
#include "MyPinCreationStateChangedMessage.h"
#include "IMetricsService.h"
#include "IAppModeModel.h"

namespace ExampleApp
{
    namespace FlattenButton
    {
        namespace View
        {
            class FlattenButtonController : private Eegeo::NonCopyable
            {
            private:
                IFlattenButtonViewModel& m_viewModel;
                IFlattenButtonView& m_view;
                ExampleAppMessaging::TMessageBus& m_messageBus;

                Eegeo::Helpers::TCallback1<FlattenButtonController, const FlattenButtonModelStateChangedMessage&> m_stateChangeHandler;
                Eegeo::Helpers::TCallback1<FlattenButtonController, bool> m_toggledCallback;
                Eegeo::Helpers::TCallback2<FlattenButtonController, ScreenControl::View::IScreenControlViewModel&, float> m_viewStateCallback;
                Eegeo::Helpers::TCallback1<FlattenButtonController, const MyPinCreation::MyPinCreationStateChangedMessage&> m_myPinCreationStateChangedMessageHandler;

                Metrics::IMetricsService& m_metricsService;
                const IAppModeModel& m_appModeModel;
                
                void OnToggleButton(bool& toggle);

                void OnFlattenButtonModelStateChangedMessage(const FlattenButtonModelStateChangedMessage& message);

                void OnViewStateChangeScreenControl(ScreenControl::View::IScreenControlViewModel& viewModel, float& state);
                
                void OnMyPinCreationStateChangedMessage(const MyPinCreation::MyPinCreationStateChangedMessage& message);

            public:
                FlattenButtonController(
                    IFlattenButtonViewModel& viewModel,
                    IFlattenButtonView& view,
                    ExampleAppMessaging::TMessageBus& messageBus,
                    Metrics::IMetricsService& metricsService,
                    const IAppModeModel& appModeModel
                );

                ~FlattenButtonController();
            };
        }
    }
}
