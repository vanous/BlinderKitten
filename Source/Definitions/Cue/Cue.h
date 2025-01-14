/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Command/Command.h"
#include "Task.h"

class Cue:
    public BaseItem
{
public:
    Cue(var params = var());
    virtual ~Cue();

    String objectType;
    var objectData;

    BaseManager<Command> commands;
    BaseManager<Task> tasks;
    FloatParameter* id;

    StringParameter* cueText;
    StringParameter* goText;

    EnumParameter* autoFollow = nullptr;
    FloatParameter* autoFollowTiming;
    FloatParameter* autoFollowCountDown;
    BoolParameter* canBeRandomlyCalled;
    BoolParameter* loadWindowBreakLine;
    double TSAutoFollowStart;
    double TSAutoFollowEnd;

    ControllableContainer actionsContainer;
    Trigger* goBtn;
    Trigger* loadBtn;
    Trigger* replaceBtn;
    Trigger* mergeBtn;
    Trigger* createAfterBtn;
    Trigger* createBeforeBtn;
    bool isComputing = false;

    HashMap<SubFixtureChannel*, std::shared_ptr<ChannelValue>> computedValues;
    CriticalSection csComputing;

    void triggerTriggered(Trigger* t) override;
    void onContainerParameterChangedInternal(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c);

    void computeValues();
    void update(double now);
    void go();
    void endTransition();

    String getTypeString() const override { return objectType; }
    static Cue* create(var params) { return new Cue(params); }
    double maxTiming = 0;
    //void updateChannels();

    FloatParameter* htpInDelay;
    FloatParameter* htpOutDelay;
    FloatParameter* ltpDelay;
    FloatParameter* htpInFade;
    FloatParameter* htpOutFade;
    FloatParameter* ltpFade;

    ControllableContainer timingContainer;

};

