/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "FixtureTypeChannel.h"
#include "FixtureTypeChannelManager.h"
#include "FixtureTypeVirtualChannelManager.h"

class FixtureType:
    public BaseItem
{
public:
    FixtureType(var params = var());
    virtual ~FixtureType();

    String objectType;
    var objectData;
    
    FixtureTypeChannelManager chansManager;
    FixtureTypeVirtualChannelManager virtualChansManager;
    String getTypeString() const override { return objectType; }

    static FixtureType* create(var params) { return new FixtureType(params); }
    void afterLoadJSONDataInternal();
    void updateVirtualLists();

};

