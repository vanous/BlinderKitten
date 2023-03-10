/*
  ==============================================================================

    VirtualFaderAction.cpp
    Created: 3 Feb 2022 10:15:35pm
    Author:  No

  ==============================================================================
*/

#include "VirtualFaderAction.h"
#include "../Cuelist/Cuelist.h"
#include "../../Brain.h"
#include "UI/VirtualFaders/VirtualFaderColGrid.h"
#include "UI/VirtualFaders/VirtualFaderColManager.h"


VirtualFaderAction::VirtualFaderAction(var params) :
    Action(params)
{
    actionType = (ActionType)(int)params.getProperty("actionType", VF_FADER);
    
    if (actionType == VF_GOTOPAGE) {
        goToPageId = addIntParameter("Target page", "go to this page", 1, 1);
    }
    else if (actionType == VF_PAGEUP || actionType == VF_PAGEDOWN) {
    }
    else {
        pageNumber = addIntParameter("Page", "Target Page 0 means current page", 0,0);
        colNumber = addIntParameter("Column", "", 1,1);
        if (actionType != VF_FADER) {
            elementNumber = addIntParameter("Number", "", 1, 1);
        }
    }
}

VirtualFaderAction::~VirtualFaderAction()
{
}

void VirtualFaderAction::triggerInternal()
{   
    //if (target == nullptr) return;
}

void VirtualFaderAction::setValueInternal(var value, String origin)
{
    float val = value;
    int number = 0;
    if (actionType == VF_PAGEUP) {
        if (val > 0) {
            VirtualFaderColGrid::getInstance()->goToPage(VirtualFaderColGrid::getInstance()->page + 1);
        }
        return;
    }
    else if(actionType == VF_PAGEDOWN) {
        if (val > 0) {
            VirtualFaderColGrid::getInstance()->goToPage(VirtualFaderColGrid::getInstance()->page - 1);
        }
        return;
    }
    else if (actionType == VF_GOTOPAGE) {
        if (val > 0) {
            VirtualFaderColGrid::getInstance()->goToPage(goToPageId->getValue());
        }
        return;
    }

    if (actionType != VF_FADER) {
        number = elementNumber->getValue();
    }
    int col = colNumber->getValue();
    int page = pageNumber->getValue();

    if (page == 0) {
        page = VirtualFaderColGrid::getInstance()->page;
    }

    bool currentPage = page == VirtualFaderColGrid::getInstance()->page;


    switch (actionType) {
    case VF_ROTARY:
        VirtualFaderColManager::getInstance()->setRotaryValue(page, col, number, val, origin);
        break;

    case VF_ABOVEBUTTON:
        VirtualFaderColManager::getInstance()->setAboveButtonValue(page, col, number, val, origin);
       break;

    case VF_FADER: {
        VirtualFaderColManager::getInstance()->setFaderValue(page, col, val, origin);
        break;
    }
    case VF_BELOWBUTTON:
        VirtualFaderColManager::getInstance()->setBelowButtonValue(page, col, number, val, origin);
        break;
    }


    /*
    page = -1;
    if (page == 0 || page == VirtualFaderColGrid::getInstance()->page) {
        page = VirtualFaderColGrid::getInstance()->page;
        const MessageManagerLock mmLock;
        if (actionType == VF_FADER) {
            if (col <= VirtualFaderColGrid::getInstance()->cols) {
                VirtualFaderSlider* vfs = VirtualFaderColGrid::getInstance()->sliderToVFS.getReference(VirtualFaderColGrid::getInstance()->faders[col - 1]);
                if (vfs == nullptr || vfs->isAllowedToMove(origin, value)) {
                    if (VirtualFaderColGrid::getInstance()->faders[col - 1]->getValue() != val) {
                        VirtualFaderColGrid::getInstance()->faders[col - 1]->setValue(val, juce::dontSendNotification);
                    }
                }
            }
            return;
        }
        else if (actionType == VF_ROTARY) {
            if (col <= VirtualFaderColGrid::getInstance()->cols) {
                if (number < VirtualFaderColGrid::getInstance()->rotaries.getRawDataPointer()[col - 1]->size()) {
                    if (VirtualFaderColGrid::getInstance()->rotaries.getRawDataPointer()[col - 1]->getRawDataPointer()[number]->getValue() != val) {
                        VirtualFaderColGrid::getInstance()->rotaries.getRawDataPointer()[col - 1]->getRawDataPointer()[number]->setValue(val);
                    }
                }
            }
            return;
        }
    }

    page = 1;
    */
    //LOG("looking for page " + String(page) + ", col" + String(col) + ", number " + String(number) + "");


    /*
    for (int i = 0; i < VirtualFaderColManager::getInstance()->items.size(); i++) {
        VirtualFaderCol* vfc = VirtualFaderColManager::getInstance()->items[i];

        if ((int)vfc->pageNumber->getValue() == page && (int)vfc->colNumber->getValue() == col) {
            switch (actionType) {
            case VF_ENCODER:
                if (vfc->rotaries.items.size() > number) {
                    VirtualFaderSlider* vb = vfc->rotaries.items[number];
                    if (vb->isAllowedToMove(origin, value)) {
                        vb->moved(value, vfc->targetType->getValue(), vfc->targetId->getValue(), origin);
                        if (currentPage && VirtualFaderColGrid::getInstance()->rotaries.size() > col && VirtualFaderColGrid::getInstance()->rotaries[col-1]->size() > number) {
                            VirtualFaderColGrid::getInstance()->rotaries[col - 1]->getRawDataPointer()[number]->setValue(val, juce::dontSendNotification);
                        }
                    }
                }

                break;

            case VF_ABOVEBUTTON:
                if (vfc->aboveButtons.items.size() > number) {
                    VirtualFaderButton* vb = vfc->aboveButtons.items[number];
                    if (val > 0) {
                        vb->pressed();
                    }
                    else {
                        vb->released();
                    }

                }

                break;

            case VF_FADER: {
                VirtualFaderSlider* vb = &vfc->fader;
                if (vb->isAllowedToMove(origin, value)) {
                    vb->moved(value, vfc->targetType->getValue(), vfc->targetId->getValue(), origin);
                    if (currentPage && VirtualFaderColGrid::getInstance()->faders.size()>col) {
                        VirtualFaderColGrid::getInstance()->faders[col - 1]->setValue(val, juce::dontSendNotification);
                    }
                }
                break;
            }
            case VF_BELOWBUTTON:
                if (vfc->belowButtons.items.size() > number) {
                    VirtualFaderButton* vb = vfc->belowButtons.items[number];
                    if (val > 0) {
                        vb->pressed();
                    }
                    else {
                        vb->released();
                    }

                }

                break;
            }
        }
        */
    
    
}