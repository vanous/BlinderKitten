/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class Task :
    public BaseItem
    // public ChainVizTarget
{
public:
    Task(var params = var());
    virtual ~Task();

    String objectType;
    var objectData;

    IntParameter* originId;

    EnumParameter* targetType;
    IntParameter* targetId;
    BoolParameter* targetThru;
    IntParameter* targetIdTo;

    EnumParameter* cuelistAction;
    EnumParameter* effectAction;
    EnumParameter* carouselAction;
    EnumParameter* mapperAction;

    FloatParameter* targetValue;

    FloatParameter* delay;
    FloatParameter* fade;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;
    void updateDisplay();

    String getTypeString() const override { return objectType; }
    static Task* create(var params) { return new Task(params); }

    static void triggerGivenTask(String targetType, int targetId, String action, double targetValue, int id);
};

