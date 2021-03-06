// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#pragma once

#include "IMetricsService.h"
#include "WindowsNativeState.h"

namespace ExampleApp
{
    namespace Metrics
    {
        class WindowsFlurryMetricsService : public IMetricsService
        {
        public:
            WindowsFlurryMetricsService(WindowsNativeState* pNativeState);

            ~WindowsFlurryMetricsService();

            void BeginSession(const std::string& apiKey, const std::string& appVersion);

            void SetEvent(const std::string& eventString);

            void SetEvent(const std::string& eventString,
                          const std::string& key, const std::string& value);

            void SetEvent(const std::string& eventString,
                          const std::string& key1, const std::string& value1,
                          const std::string& key2, const std::string& value2);

            void SetEvent(const std::string& eventString,
                          const std::string& key1, const std::string& value1,
                          const std::string& key2, const std::string& value2,
                          const std::string& key3, const std::string& value3);

            void BeginTimedEvent(const std::string& eventString);
            void BeginTimedEvent(const std::string& eventString, const std::string& key, const std::string& value);

            void EndTimedEvent(const std::string& eventString);
            void EndTimedEvent(const std::string& eventString, const std::string& key, const std::string& value);

            void SetPosition(const double latitude, const double longitude, const double horizontalAccuracy, const double verticalAccuracy);

            bool IsEnabled() const;

        private:
            WindowsNativeState* m_pNativeState;
            //jclass m_flurryClass;
            bool m_enabled;
        };
    }
}
