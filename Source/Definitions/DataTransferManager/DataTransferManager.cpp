/*
  ==============================================================================

    DataTransferManager.cpp
    Created: 29 Jan 2019 3:52:46pm
    Author:  no

  ==============================================================================
*/
#include "JuceHeader.h"
#include "DataTransferManager.h"
#include "../ChannelValue.h"
#include "../../Brain.h"
#include "../SubFixture/SubFixture.h"
#include "../Group/GroupManager.h"
#include "../Group/Group.h"
#include "../Preset/PresetManager.h"
#include "../Preset/Preset.h"
#include "../Cuelist/CuelistManager.h"
#include "../Cuelist/Cuelist.h"
#include "../Cue/Cue.h"
#include "../Programmer/Programmer.h"
#include "../Command/CommandSelectionManager.h"
#include "../Command/CommandSelection.h"
#include "../ChannelFamily/ChannelFamilyManager.h"
#include "../ChannelFamily/ChannelFamily.h"
#include "../TimingPreset/TimingPresetManager.h"
#include "../TimingPreset/TimingPreset.h"
#include "../Effect/EffectManager.h"
#include "../Effect/Effect.h"
#include "../Carousel/CarouselManager.h"
#include "../Carousel/Carousel.h"
#include "../Fixture/FixtureManager.h"
#include "../Fixture/Fixture.h"

juce_ImplementSingleton(DataTransferManager)

DataTransferManager::DataTransferManager() :
	BaseItem("Data Transfer Manager")
{
    sourceType = addEnumParameter("Source Type", "Type of the data source");
    sourceType->addOption("Group", "group");
    sourceType->addOption("Preset", "preset");
    // sourceType->addOption("Timing Preset", "TimingPreset");
    sourceType->addOption("Cuelist", "cuelist");
    sourceType->addOption("Programmer", "programmer");
    sourceId = addIntParameter("Source Id", "ID of the source", 0, 0);
    targetType = addEnumParameter("Target Type", "Type of the data target");
    targetType->addOption("Group", "group");
    targetType->addOption("Preset", "preset");
    // targetType->addOption("Timing Preset", "TimingPreset");
    targetType->addOption("Cuelist", "cuelist");
    targetType->addOption("Programmer", "programmer");

    targetUserId = addIntParameter("Target Id", "ID of the target", 0, 0);

    paramfilter = addTargetParameter("Param filter", "Filter recorded values in preset by parameter family", ChannelFamilyManager::getInstance());
    paramfilter->targetType = TargetParameter::CONTAINER;
    paramfilter->maxDefaultSearchLevel = 0;

    groupCopyMode = addEnumParameter("Group merge mode", "Group record mode");
    groupCopyMode->addOption("Merge", "merge");
    groupCopyMode->addOption("Replace", "replace");

    presetCopyMode = addEnumParameter("Preset merge mode", "Preset record mode");
    presetCopyMode->addOption("Merge", "merge");
    presetCopyMode->addOption("Replace", "replace");

    cuelistCopyMode = addEnumParameter("Cuelist merge mode", "Cuelist record mode");
    cuelistCopyMode->addOption("Add new cue", "add");
    cuelistCopyMode->addOption("Update current cue", "update");
    cuelistCopyMode->addOption("Replace current cue", "replace");

    go = addTrigger("Transfer Data", "Run the data transfer");
    updateDisplay();
}

void DataTransferManager::updateDisplay() {
    String tgt = targetType->getValue();
    paramfilter->hideInEditor = !(tgt == "Preset");
    groupCopyMode->hideInEditor = !(tgt == "Group");
    presetCopyMode->hideInEditor = !(tgt == "Preset");
    cuelistCopyMode->hideInEditor = !(tgt == "Cuelist");
    queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

void DataTransferManager::onContainerParameterChangedInternal(Parameter* p) {
    updateDisplay();
}

DataTransferManager::~DataTransferManager()
{
}

void DataTransferManager::triggerTriggered(Trigger* t) {
    if (t == go) {
        execute();
    }
}


void DataTransferManager::execute() {
    String srcType = sourceType->getValue();
    String trgType = targetType->getValue();
    bool valid = false;

    int tId = targetUserId->getValue();

    if (srcType == "programmer") {
        Programmer* source = Brain::getInstance()->getProgrammerById(sourceId->getValue());
        if (source == nullptr) { LOG("Invalid Programmer ID"); return; }
        if (trgType == "group") {
            valid = true;
            Group* target = Brain::getInstance()->getGroupById(tId);
            if (target == nullptr) {
                target = GroupManager::getInstance()->addItem(new Group());
                target->id->setValue(tId);
                target->setNiceName("Group " + String(int(target->id->getValue())));
            }

            if (groupCopyMode->getValue() == "replace") {
                target->selection.clear(); // erase data
            }

            for (int commandIndex = 0; commandIndex < source->commands.items.size(); commandIndex++) {
                CommandSelectionManager* selections = &source->commands.items[commandIndex]->selection;
                for (int selectionIndex = 0; selectionIndex < selections->items.size(); selectionIndex++) {
                    CommandSelection* selection = selections->items[selectionIndex];
                    CommandSelection* newSel = target->selection.addItem();
                    newSel->loadJSONData(selection->getJSONData());
                }
            }
        }
        else if (trgType == "preset") {
            valid = true;
            Preset* target = Brain::getInstance()->getPresetById(tId);
            if (target == nullptr) {
                target = PresetManager::getInstance()->addItem(new Preset());
                target->id->setValue(tId);
                target->setNiceName("Preset " + String(int(target->id->getValue())));
                target->updateName();
                target->subFixtureValues.clear();
            }

            if (presetCopyMode->getValue() == "replace") {
                target->subFixtureValues.clear(); // erase data
            }

            ChannelFamily* filter = dynamic_cast<ChannelFamily*>(paramfilter->targetContainer.get());

            source->computeValues();
            for (auto it = source->computedValues.begin(); it != source->computedValues.end(); it.next()) {
                // HashMap<SubFixtureChannel*, ChannelValue*> computedValues;
                SubFixtureChannel* chan = it.getKey();
                ChannelValue* cValue = it.getValue();
                ChannelFamily* chanType = dynamic_cast<ChannelFamily*>(chan->channelType->parentContainer->parentContainer.get());

                if (cValue->endValue != -1 && (filter == nullptr || filter == chanType)) {

                    int subfixtId = chan->parentSubFixture->subId;
                    int fixtId = dynamic_cast<Fixture*>(chan->parentSubFixture->parentFixture)->id->getValue();
                    PresetSubFixtureValues* pfv = nullptr;

                    for (int i = 0; i < target->subFixtureValues.items.size(); i++) {
                        PresetSubFixtureValues* temp = target->subFixtureValues.items[i];
                        if ((int)temp->targetFixtureId->getValue() == fixtId && (int)temp->targetSubFixtureId->getValue() == subfixtId) {
                            pfv = target->subFixtureValues.items[i];
                        }
                    }
                    if (pfv == nullptr) {
                        pfv = target->subFixtureValues.addItem();
                        pfv->targetFixtureId->setValue(fixtId);
                        pfv->targetSubFixtureId->setValue(subfixtId);
                        pfv->values.clear();
                    }

                    PresetValue* pv = nullptr;
                    for (int i = 0; i < pfv->values.items.size(); i++) {
                        if (dynamic_cast<ChannelType*>(pfv->values.items[i]->param->targetContainer.get()) == chan->channelType) {
                            pv = pfv->values.items[i];
                        }
                    }
                    if (pv == nullptr) {
                        pv = pfv->values.addItem();
                        pv->param->setValueFromTarget(chan->channelType);
                    }
                    pv->paramValue->setValue(cValue->endValue);
                }
            target->updateDisplay();
            }
        }
        else if (trgType == "cuelist") {
            valid = true;
            Cuelist* target = Brain::getInstance()->getCuelistById(tId);
            if (target == nullptr) {
                target = CuelistManager::getInstance()->addItem(new Cuelist());

                target->id->setValue(tId);
                target->userName->setValue("Cuelist "+target->id->getValue().toString());
                target->cues.clear();
            }

            String copyMode = cuelistCopyMode->getValue();
            Cue* targetCue;

            if (copyMode == "add") {
                targetCue = target->cues.addItem();
                targetCue->commands.clear();
            }
            else {
                targetCue = target->cueA;
                if (targetCue == nullptr) {
                    targetCue = target->cues.items[0];
                    if (targetCue == nullptr){
                        targetCue = target->cues.addItem();
                        targetCue->commands.clear();
                    }
                }
            }
            if (copyMode == "replace") {
                targetCue->commands.clear();
            }

            for (int i = 0; i < source->commands.items.size(); i++) {
                Command* c = targetCue->commands.addItem();
                c->loadJSONData(source->commands.items[i]->getJSONData());
            }
        }

    }


    else if (srcType == "cuelist") {

    }
    else if (srcType == "preset") {

    }
    else if (srcType == "group") {

    }


    if (!valid) {
        LOGWARNING("target type and source type are not compatible");
    }
    else {
    }


}



void DataTransferManager::editObject(String type, int id) {
    if (type == "fixture") {
        Fixture* f = Brain::getInstance()->getFixtureById(id);
        //if (f == nullptr) { f = new Fixture(); f->id->setValue(id); }
        if (f != nullptr) {
            f->selectThis();
        }
        else {
            LOGERROR("Fixture " + String(id) + " does not exists");
        }
    }
    else if (type == "group") {
        Group* target = Brain::getInstance()->getGroupById(id);
        if (target == nullptr) { target = GroupManager::getInstance()->addItem(); target->id->setValue(id); }
        target->selectThis();
    }
    else if (type == "preset") {
        Preset* target = Brain::getInstance()->getPresetById(id);
        if (target == nullptr) { target = PresetManager::getInstance()->addItem(); target->id->setValue(id); }
        target->selectThis();
    }
    else if (type == "cuelist") {
        Cuelist* target = Brain::getInstance()->getCuelistById(id);
        if (target == nullptr) { target = CuelistManager::getInstance()->addItem(); target->id->setValue(id); }
        target->selectThis();
    }
    else if (type == "effect") {
        Effect* target = Brain::getInstance()->getEffectById(id);
        if (target == nullptr) { target = EffectManager::getInstance()->addItem(); target->id->setValue(id); }
        target->selectThis();
    }
    else if (type == "carousel") {
        Carousel* target = Brain::getInstance()->getCarouselById(id);
        if (target == nullptr) { target = CarouselManager::getInstance()->addItem(); target->id->setValue(id); }
        target->selectThis();
    }
}

void DataTransferManager::deleteObject(String type, int id) {
    if (type == "fixture") {
        Fixture* target = Brain::getInstance()->getFixtureById(id);
        if (target != nullptr) { FixtureManager::getInstance()->removeItem(target); }
    }
    else if (type == "group") {
        Group* target = Brain::getInstance()->getGroupById(id);
        if (target != nullptr) { GroupManager::getInstance()->removeItem(target); }
    }
    else if (type == "preset") {
        Preset* target = Brain::getInstance()->getPresetById(id);
        if (target != nullptr) { PresetManager::getInstance()->removeItem(target); }
    }
    else if (type == "cuelist") {
        Cuelist* target = Brain::getInstance()->getCuelistById(id);
        if (target != nullptr) { CuelistManager::getInstance()->removeItem(target); }
    }
    else if (type == "effect") {
        Effect* target = Brain::getInstance()->getEffectById(id);
        if (target != nullptr) { EffectManager::getInstance()->removeItem(target); }
    }
    else if (type == "carousel") {
        Carousel* target = Brain::getInstance()->getCarouselById(id);
        if (target != nullptr) { CarouselManager::getInstance()->removeItem(target); }
    }
}

void DataTransferManager::moveObject(String type, int id, int idTo) {
    if (type == "fixture") {
        Fixture* source = Brain::getInstance()->getFixtureById(id);
        Fixture* target = Brain::getInstance()->getFixtureById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
    else if (type == "group") {
        Group* source = Brain::getInstance()->getGroupById(id);
        Group* target = Brain::getInstance()->getGroupById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
    else if (type == "preset") {
        Preset* source = Brain::getInstance()->getPresetById(id);
        Preset* target = Brain::getInstance()->getPresetById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
    else if (type == "cuelist") {
        Cuelist* source = Brain::getInstance()->getCuelistById(id);
        Cuelist* target = Brain::getInstance()->getCuelistById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
    else if (type == "effect") {
        Effect* source = Brain::getInstance()->getEffectById(id);
        Effect* target = Brain::getInstance()->getEffectById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
    else if (type == "carousel") {
        Carousel* source = Brain::getInstance()->getCarouselById(id);
        Carousel* target = Brain::getInstance()->getCarouselById(idTo);
        if (target != nullptr) { target->id->setValue(99999999); }
        source->id->setValue(idTo);
        if (target != nullptr) { target->id->setValue(id); }
    }
}