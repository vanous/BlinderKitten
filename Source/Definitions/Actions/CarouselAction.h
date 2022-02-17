/*
  ==============================================================================

    CuelistAction.h
    Created: 3 Feb 2022 10:15:35pm
    Author:  No

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include"../../Common/Action/Action.h"

class CarouselAction :
    public Action
{
public:
    CarouselAction(var params);
    ~CarouselAction();

    enum ActionType { CAR_START, CAR_STOP, CAR_SIZE, CAR_SPEED, CAR_TAPTEMPO };
    ActionType actionType;
    IntParameter* targetId;
    FloatParameter* maxSpeed;

    void triggerInternal() override;
    void setValueInternal(var value);

    static CarouselAction* create(var params) { return new CarouselAction(params); }

};