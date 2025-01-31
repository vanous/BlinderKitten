/*
  ==============================================================================
 
    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "CommandValue.h"
#include "CommandTiming.h"
#include "CommandSelectionManager.h"

class CommandSelectionManager;

class CommandTiming;
class ChannelValue;
class SubFixtureChannel;
class Cuelist;
class Cue;
class CommandSelection;
class ChannelType;

class Command:
    public BaseItem
{
public:
    Command(var params = var());
    virtual ~Command();

    String objectType;
    var objectData;

    TargetParameter* devTypeParam;

    CommandSelectionManager selection;
    BaseManager<CommandValue> values;
    CommandTiming timing;

    HashMap<SubFixtureChannel*, std::shared_ptr<ChannelValue>> computedValues;
    CriticalSection isComputing;
    bool toDelete = false;

    String getTypeString() const override { return objectType; }
    void parameterValueChanged(Parameter* p);
    void afterLoadJSONDataInternal();
    void updateDisplay();

    double maxTiming;
    void computeValues();
    void computeValues(Cuelist* cl, Cue* c);

    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) override;
    void triggerTriggered(Trigger* t);
    static Command* create(var params) { return new Command(params); }

    StringArray getCommandAsTexts();
    StringArray getCommandSelectionAsTexts();
    //Trigger* viewCommandBtn;
    Trigger* explodeSelectionBtn;
    String formatValue(float f);

    bool userCanPressPlusOrMinus = true;
    bool userCanPressSelectionType = true;
    bool userCanPressSubSelection = false;
    bool userCanPressValueType = false;
    bool userCanPressTimingType = false;
    bool userCanPressThru = false;
    bool userCanPressNumber = false;
    bool userCanPressValue = false;
    bool userCanPressSym = false;
    bool userCanHaveAnotherCommand = false;
    void userCantPress();
    void userPress(String s);

    CommandSelection* currentUserSelection = nullptr;
    CommandValue* currentUserValue = nullptr;
    Parameter* currentUserTarget = nullptr;
    BoolParameter* currentUserThru = nullptr;

    float getChannelValue(ChannelType* t, bool thru = false);
    String lastTarget = "";

    void explodeSelection();

};

