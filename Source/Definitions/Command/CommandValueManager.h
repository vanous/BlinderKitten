/*
  ==============================================================================

    ObjectManager.h
    Created: 26 Sep 2020 10:02:28am
    Author:  bkupe

  ==============================================================================
*/

#pragma once
#include "CommandValue.h"
// #include "../ChannelFamily/ChannelType/ChannelType.h"

class CommandValueManager :
    public BaseManager<CommandValue>
{
public:
    CommandValueManager();
    ~CommandValueManager();
};

