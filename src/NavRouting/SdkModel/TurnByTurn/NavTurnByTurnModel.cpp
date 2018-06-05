// Copyright eeGeo Ltd (2012-2015), All Rights Reserved

#include "NavTurnByTurnModel.h"
#include "RouteHelpers.h"
#include "ILocationService.h"
#include "INavRoutingModel.h"
#include "Point3Spline.h"

namespace ExampleApp
{
    namespace NavRouting
    {
        namespace SdkModel
        {
            namespace TurnByTurn
            {
                namespace
                {
                    double CalculateDuration(double t, double duration)
                    {
                        return (1.0-Eegeo::Math::Clamp01(t))*duration;
                    }

                    double CalculateDistanceToEndOfStep(const Eegeo::Routes::Webservice::RouteStep& step, double paramAlongStep)
                    {
                        return step.Distance * (1.0 - Eegeo::Math::Clamp01(paramAlongStep));
                    }
                }

                NavTurnByTurnModel::NavTurnByTurnModel(const NavTurnByTurnConfig &config,
                                                       Eegeo::Location::ILocationService &locationService)
                : m_config(config)
                , m_locationService(locationService)
                , m_closestPointOnRoute(0,0)
                , m_enabled(false)
                , m_remainingDuration(0.0)
                , m_currentStepIndex(0)
                , m_paramAlongStep(0.0)
                , m_paramAlongRoute(0.0)
                , m_distanceFromRoute(0.0)
                , m_indexOfPathSegmentStartVertex(0)
                , m_updateTime(0.0f)
                {
                }

                NavTurnByTurnModel::~NavTurnByTurnModel()
                {
                }

                void NavTurnByTurnModel::Update(float dt) {

                    if(!TurnByTurnEnabled())
                    {
                        return;
                    }

                    m_updateTime += dt;

                    if(m_updateTime >= m_config.updateRateSeconds) {
                        m_updateTime = 0;
                        UpdateTurnByTurn();
                    }
                }

                void NavTurnByTurnModel::UpdateTurnByTurn() {

                    Eegeo::Space::LatLong currentLocation(0,0);
                    bool gotLocation = m_locationService.GetLocation(currentLocation);
                    if(!gotLocation)
                    {
                        // TODO: Handle failure to get current location
                        return;
                    }

                    // TODO: Check this actually works if you're not looking at the indoor model itself.
                    Eegeo::Routes::PointOnRouteOptions options;
                    if(m_locationService.IsIndoors())
                    {
                        Eegeo::Resources::Interiors::InteriorId indoorId = m_locationService.GetInteriorId();
                        options.IndoorMapId = indoorId.Value();

                        int floorIndex;
                        m_locationService.GetFloorIndex(floorIndex);
                        // TODO: FloorIndex != FloorID.  Check how to convert these
                        // (Might require Indoor model which only exists if you're in it)
                        options.IndoorMapFloorId = floorIndex;
                    }

                    Eegeo::Routes::PointOnRoute pointOnRouteResult = Eegeo::Routes::RouteHelpers::GetPointOnRoute(currentLocation, m_route, options);

                    double distanceToRouteAtCurrentPoint = pointOnRouteResult.GetPointOnPathForClosestRouteStep().GetDistanceFromInputPoint();
                    bool shouldReroute = distanceToRouteAtCurrentPoint > m_config.distanceToPathToTriggerReroute;
                    if(shouldReroute)
                    {
                        m_shouldRerouteCallbacks.ExecuteCallbacks();
                    }

                    // TODO: First test is just use the results from here. Actually need to decide at what threshold to advance to next point
                    int nextSectionIndex = pointOnRouteResult.GetRouteSectionIndex();
                    int nextStepIndex = pointOnRouteResult.GetRouteStepIndex();
                    bool isOnCurrentOrNextStep = nextSectionIndex >= m_currentSectionIndex && nextStepIndex >= m_currentStepIndex;
                    if(!isOnCurrentOrNextStep) {
                        return;
                    }

                    bool tooFarFromPath = distanceToRouteAtCurrentPoint > m_config.distanceToPathRangeMeters;
                    if(tooFarFromPath) {
                        return;
                    }

                    m_closestPointOnRoute = pointOnRouteResult.GetPointOnPathForClosestRouteStep().GetResultPoint();
                    m_distanceFromRoute = pointOnRouteResult.GetPointOnPathForClosestRouteStep().GetDistanceFromInputPoint();

                    // NOTE: Duration is currently just a fraction of the total route duration from the service / your progress.
                    m_remainingDuration = CalculateDuration(pointOnRouteResult.GetFractionAlongRoute(), m_route.Duration);

                    m_currentSectionIndex = nextSectionIndex;
                    m_currentStepIndex = nextStepIndex;

                    m_paramAlongStep = pointOnRouteResult.GetFractionAlongRouteStep();
                    m_paramAlongRoute = pointOnRouteResult.GetFractionAlongRoute();
                    const Eegeo::Routes::Webservice::RouteSection& currentSection = m_route.Sections.at(
                            static_cast<unsigned long>(m_currentSectionIndex));
                    const Eegeo::Routes::Webservice::RouteStep& currentStep = currentSection.Steps.at(
                            static_cast<unsigned long>(m_currentStepIndex));

                    m_distanceToNextStep = CalculateDistanceToEndOfStep(currentStep, pointOnRouteResult.GetFractionAlongRouteStep());

                    m_indexOfPathSegmentStartVertex = pointOnRouteResult.GetPointOnPathForClosestRouteStep().GetIndexOfPathSegmentStartVertex();
                    
                    m_updateCallbacks.ExecuteCallbacks();
                }

                void NavTurnByTurnModel::Start(const Eegeo::Routes::Webservice::RouteData& route)
                {
                    if(TurnByTurnEnabled()) {
                        return;
                    }

                    m_route = route;
                    m_distanceFromRoute = 0.0;
                    m_remainingDuration = route.Duration;
                    m_distanceToNextStep = 0;
                    m_currentSectionIndex = 0;
                    m_currentStepIndex = 0;
                    m_paramAlongRoute = 0.0;
                    m_paramAlongStep = 0.0;
                    m_updateTime = 0.0f;
                    m_indexOfPathSegmentStartVertex = 0;
                    m_enabled = true;

                    m_startedCallbacks.ExecuteCallbacks();

                    UpdateTurnByTurn();
                }

                void NavTurnByTurnModel::Stop()
                {
                    if(TurnByTurnEnabled())
                    {
                        m_enabled = false;

                        m_stoppedCallbacks.ExecuteCallbacks();
                    }
                }

                void NavTurnByTurnModel::InsertStartedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_startedCallbacks.AddCallback(callback);
                }

                void NavTurnByTurnModel::RemoveStartedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_startedCallbacks.RemoveCallback(callback);
                }

                void NavTurnByTurnModel::InsertStoppedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_stoppedCallbacks.AddCallback(callback);
                }

                void NavTurnByTurnModel::RemoveStoppedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_stoppedCallbacks.RemoveCallback(callback);
                }

                void NavTurnByTurnModel::InsertUpdatedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_updateCallbacks.AddCallback(callback);
                }

                void NavTurnByTurnModel::RemoveUpdatedCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_updateCallbacks.RemoveCallback(callback);
                }

                void NavTurnByTurnModel::InsertShouldRerouteCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_shouldRerouteCallbacks.AddCallback(callback);
                }

                void NavTurnByTurnModel::RemoveShouldRerouteCallback(Eegeo::Helpers::ICallback0& callback)
                {
                    m_shouldRerouteCallbacks.RemoveCallback(callback);
                }
            }
        }
    }
}