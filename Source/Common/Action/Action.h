/*
  ==============================================================================

    Action.h
    Created: 8 Oct 2020 3:33:09pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Action :
    public BaseItem
{
public:
    Action(var params = var());
    virtual ~Action();

    String typeString;
    var previousValue = 0;

    void trigger();
    virtual void triggerInternal() {}

    void setValue(var value);
    void setValue(var value, String origin, bool isRelative);
    virtual void setValueInternal(var value) {setValueInternal(value, "unknown", false); }
    virtual void setValueInternal(var value, String origin, bool isRelative) {}
    virtual var getValue() {return var(); };

    String getTypeString() const override { return typeString; }
};