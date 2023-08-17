/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "BKPath/BKPath.h"

class SubFixture;
class Fixture;

class Layout :
    public ChangeBroadcaster,
    public BaseItem
    // public ChainVizTarget
{
public:
    Layout(var params = var());
    virtual ~Layout();

    String objectType;
    var objectData;

    HashMap<Fixture*, std::shared_ptr<Vector3D<float>>> fixtToPos;
    HashMap<SubFixture*, std::shared_ptr<Vector3D<float>>> subFixtToPos;
    CriticalSection isComputing;

    IntParameter* id;
    int registeredId = 0;
    StringParameter* userName;
    void updateName();

    Point2DParameter* dimensionsX;
    Point2DParameter* dimensionsY;

    BaseManager<BKPath> paths;
    StringParameter* categoryName;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

    void computeData();

    String getTypeString() const override { return objectType; }
    static Layout* create(var params) { return new Layout(params); }
};

