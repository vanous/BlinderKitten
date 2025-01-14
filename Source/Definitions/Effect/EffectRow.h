/*
  ==============================================================================

    ChannelType.h
    Created: 7 Nov 2021 7:40:48pm
    Author:  No

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Command/CommandSelectionManager.h"
#include "EffectParam.h"

class EffectRow:
    public BaseItem
{
    public:
    EffectRow(var params = var());
    ~EffectRow();

    String objectType;
    var objectData;

    ControllableContainer curveContainer;
    BaseManager<EffectParam> paramContainer;

    EnumParameter* curvePresetOrValue;
    Automation curve;
    FloatParameter* curveOrigin;
    IntParameter* presetId;
    FloatParameter* chaserFade;
    IntParameter* chaserBuddying;
    FloatParameter* speed;
    EnumParameter* direction;

    CommandSelectionManager selection;

    HashMap<SubFixture*, double> computedPositions;
    CriticalSection isComputing;

    void computeData();

    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*);
    void updateDisplay();


};