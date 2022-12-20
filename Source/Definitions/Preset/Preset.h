/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "PresetSubFixtureValues.h"
#include "PresetValue.h"
#include "../SubFixture/SubFixture.h"
#include "../ChannelFamily/ChannelType/ChannelType.h"
#include "../FixtureType/FixtureType.h"

class Preset:
    public BaseItem
{
public:
    Preset(var params = var());
    virtual ~Preset();

    String objectType;
    var objectData;

    IntParameter* id;
    int registeredId = 0;
    StringParameter* userName;
    void onContainerParameterChangedInternal(Parameter* p);
    void updateName();

    TargetParameter* devTypeParam;
    EnumParameter* presetType;
    Trigger* testMeButton;
    Trigger* reOrderButton;

    HashMap<SubFixture*, HashMap<ChannelType*, float>*> computedSubFixtureValues;
    HashMap<FixtureType*, HashMap<ChannelType*, float>*> computedFixtureTypeValues;
    HashMap<FixtureType*, HashMap<String, HashMap<ChannelType*, float>*>*> computedSubFixtureTypeValues;
    HashMap<ChannelType*, float> computedUniversalValues;

    HashMap<ChannelType*, float>* getSubFixtureValues(SubFixture* f);

    Trigger* loadToProgrammerBtn;
    void triggerTriggered(Trigger* t) override;

    BaseManager<PresetSubFixtureValues> subFixtureValues;
    String getTypeString() const override { return objectType; }

    static Preset* create(var params) { return new Preset(params); }

    void computeValues();
    void updateDisplay();

    void reorderPresetContent();

    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c);


};
