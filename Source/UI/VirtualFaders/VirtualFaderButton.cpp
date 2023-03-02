/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "VirtualFaderButton.h"
#include "VirtualFaderColManager.h"
#include "VirtualFaderColGrid.h"
#include "../../Brain.h"

VirtualFaderButton::VirtualFaderButton(var params) :
	BaseItem(params.getProperty("name", "VirtualFaderButton")),
	objectType(params.getProperty("type", "VirtualFaderButton").toString()),
	objectData(params),
	actionManager("Generic Actions")
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = false;

	itemDataType = "VirtualFaderButton";

	targetType = addEnumParameter("Target type", "");
	targetType->addOption("disabled", "disabled");
	targetType->addOption("Same as column", "column");
	targetType->addOption("Cuelist", "cuelist");
	targetType->addOption("Effect", "effect");
	targetType->addOption("Carousel", "carousel");
	targetType->addOption("Mapper", "mapper");
	targetType->addOption("Generic Actions", "actions");

	targetId = addIntParameter("Target ID", "", 0, 0);
	cuelistAction = addEnumParameter("Cuelist action", "");
	cuelistAction->addOption("Go", "go");
	cuelistAction->addOption("Go back", "goback");
	cuelistAction->addOption("Off", "off");
	cuelistAction->addOption("Toggle", "toggle");
	cuelistAction->addOption("Flash", "flash");
	cuelistAction->addOption("Load", "load");
	cuelistAction->addOption("Load and Go", "loadandgo");
	cuelistAction->addOption("Swop", "swop");
	cuelistAction->addOption("Go random", "gorandom");

	cueId = addFloatParameter("Cue ID", "Insert here the id of the cue you want to load, -1 will prompt the cue choose window", -1, -1);

	effectAction = addEnumParameter("Effect action", "");
	effectAction->addOption("Start", "start");
	effectAction->addOption("Stop", "stop");
	effectAction->addOption("Toggle", "toggle");
	effectAction->addOption("Tap tempo", "taptempo");
	effectAction->addOption("Double Speed", "doublespeed");
	effectAction->addOption("Half Speed", "halfspeed");

	carouselAction = addEnumParameter("Carousel Action", "");
	carouselAction->addOption("Start", "start");
	carouselAction->addOption("Stop", "stop");
	carouselAction->addOption("Toggle", "toggle");
	carouselAction->addOption("Tap tempo", "taptempo");
	carouselAction->addOption("Double Speed", "doublespeed");
	carouselAction->addOption("Half Speed", "halfspeed");

	mapperAction = addEnumParameter("Mapper Action", "");
	mapperAction->addOption("Start", "start");
	mapperAction->addOption("Stop", "stop");
	mapperAction->addOption("Toggle", "toggle");

	addChildControllableContainer(&actionManager);

	// id = addIntParameter("ID", "ID of this VirtualFaderButton", 1, 1);
	// userName = addStringParameter("Name", "Name of this VirtualFaderButton","New VirtualFaderButton");
	updateDisplay();
}

VirtualFaderButton::~VirtualFaderButton()
{
	VirtualFaderColGrid::getInstance()->buttonToVFB.removeValue(this);
}

void VirtualFaderButton::updateName() {
}

void VirtualFaderButton::onContainerParameterChangedInternal(Parameter* c) {
	if (c == targetType || c == cuelistAction) {
		updateDisplay();
	}
	VirtualFaderColGrid::getInstance()->fillCells();
}

void VirtualFaderButton::updateDisplay() {
	String targType = targetType->getValue();
	
	targetId->hideInEditor = targType == "column" || targType == "disabled" || targType == "actions";

	if (targType == "column" && parentContainer != nullptr) {
		targType = dynamic_cast<VirtualFaderCol*>(parentContainer->parentContainer.get())->targetType->getValue();
	}
	cuelistAction->hideInEditor = targType != "cuelist";
	effectAction->hideInEditor = targType != "effect";
	carouselAction->hideInEditor = targType != "carousel";
	mapperAction->hideInEditor = targType != "mapper";

	actionManager.hideInEditor = targType != "actions";

	bool isLoad = targType == "cuelist";
	isLoad = isLoad && (cuelistAction->getValue() == "load" || cuelistAction->getValue() == "loadandgo");

	cueId->hideInEditor = !isLoad;

	queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
}

void VirtualFaderButton::pressed(String colTargetType, int colTargetId) {
	String targType = targetType->getValue();
	if (targType == "actions") {
		actionManager.setValueAll(1);
		return;
	}
	int targId = targetId->getValue();
	
	if (targType == "column") {
		targType = colTargetType;
		targId = colTargetId;
	}

	if (targId == 0) {return;}

	if (targType == "cuelist") {
		Cuelist* targ = Brain::getInstance()->getCuelistById(targId);
		if (targ != nullptr) {
			String action = cuelistAction->getValue();
			if (action == "go") { targ->userGo(); }
			if (action == "goback") { targ->goBack(); }
			if (action == "off") { targ->off(); }
			if (action == "toggle") { targ->toggle(); }
			if (action == "flash") { targ->flash(true, false, false); }
			if (action == "load") {
				float targetCue = cueId->floatValue();
				if (targetCue == -1) {
					targ->showLoad();
				}
				else {
					targ->nextCueId->setValue(targetCue);
				}
			}
			if (action == "loadandgo") {
				float targetCue = cueId->floatValue();
				if (targetCue == -1) {
					targ->showLoadAndGo();
				}
				else {
					targ->nextCueId->setValue(targetCue);
					targ->userGo();
				}
			}
			if (action == "swop") { targ->flash(true, false, true); }
			if (action == "gorandom") { targ->goRandom(); }
		}
	}
	else if (targType == "effect") {
		Effect* targ = Brain::getInstance()->getEffectById(targId);
		if (targ != nullptr) {
			String action = effectAction->getValue();
			if (action == "start") { targ->start(); }
			if (action == "stop") { targ->stop(); }
			if (action == "toggle") {
				if (targ->isOn) {
					targ->stop();
				}
				else {
					targ->start();
				}
			}
			if (action == "taptempo") { targ->tapTempo(); }
			if (action == "doublespeed") { targ->speed->setValue((double)targ->speed->getValue() * 2); }
			if (action == "halfspeed") { targ->speed->setValue((double)targ->speed->getValue() / 2); }
		}
	}
	else if (targType == "carousel") {
		Carousel* targ = Brain::getInstance()->getCarouselById(targId);
		if (targ != nullptr) {
			String action = carouselAction->getValue();
			if (action == "start") { targ->start(); }
			if (action == "stop") { targ->stop(); }
			if (action == "toggle") {
				if (targ->isOn) {
					targ->stop();
				}
				else {
					targ->start();
				}
			}
			if (action == "taptempo") { targ->tapTempo(); }
			if (action == "doublespeed") { targ->speed->setValue((double)targ->speed->getValue() * 2); }
			if (action == "halfspeed") { targ->speed->setValue((double)targ->speed->getValue() / 2); }
		}
	}
	else if (targType == "mapper") {
		Mapper* targ = Brain::getInstance()->getMapperById(targId);
		if (targ != nullptr) {
			String action = mapperAction->getValue();
			if (action == "start") { targ->start(); }
			if (action == "stop") { targ->stop(); }
			if (action == "toggle") {
				if (targ->isOn) {
					targ->stop();
				}
				else {
					targ->start();
				}
			}
		}
	}
}

void VirtualFaderButton::released(String colTargetType, int colTargetId) {
	String targType = targetType->getValue();
	if (targType == "actions") {
		actionManager.setValueAll(0);
		return;
	}
	int targId = targetId->getValue();

	if (targType == "column") {
		targType = colTargetType;
		targId = colTargetId;
	}

	if (targId == 0) { return; }

	if (targType == "cuelist") {
		Cuelist* targ = Brain::getInstance()->getCuelistById(targId);
		if (targ != nullptr) {
			String action = cuelistAction->getValue();
			if (action == "flash") { targ->flash(false, false, false); }
			if (action == "swop") { targ->flash(false, false, true); }
		}
	}
	else if (targType == "effect") {
		Effect* targ = Brain::getInstance()->getEffectById(targId);
		if (targ != nullptr) {
			String action = effectAction->getValue();
			// if (action == "start") { targ->start(); }
		}
	}
	else if (targType == "carousel") {
		Carousel* targ = Brain::getInstance()->getCarouselById(targId);
		if (targ != nullptr) {
			String action = carouselAction->getValue();
			// if (action == "start") { targ->start(); }
		}
	}
	else if (targType == "mapper") {
		Mapper* targ = Brain::getInstance()->getMapperById(targId);
		if (targ != nullptr) {
			String action = mapperAction->getValue();
			// if (action == "start") { targ->start(); }
		}
	}

}

String VirtualFaderButton::getBtnText(String columnType) {
	String text = "";
	String targType = targetType->getValue();
	String action = "";
	if (targType == "column" && columnType != "") {
		targType = columnType;
		if (targType == "cuelist") {
			action = cuelistAction->getValue();
		}
		else if (targType == "effect") {
			action = effectAction->getValue();
		}
		else if (targType == "carousel") {
			action = carouselAction->getValue();
		}
		else if (targType == "mapper") {
			action = mapperAction->getValue();
		}

		return action;
	}
	else {
		int targId = targetId->getValue();
		if (targId == 0) { return ""; }

		if (targType == "cuelist") {
			Cuelist* targ = Brain::getInstance()->getCuelistById(targId);
			action = cuelistAction->getValue();
			if (targ != nullptr) {
				text = targ->userName->getValue();
			}
		}
		else if (targType == "effect") {
			Effect* targ = Brain::getInstance()->getEffectById(targId);
			action = effectAction->getValue();
			if (targ != nullptr) {
				text = targ->userName->getValue();
			}
		}
		else if (targType == "carousel") {
			Carousel* targ = Brain::getInstance()->getCarouselById(targId);
			action = carouselAction->getValue();
			if (targ != nullptr) {
				text = targ->userName->getValue();
			}
		}
		else if (targType == "mapper") {
			Mapper* targ = Brain::getInstance()->getMapperById(targId);
			action = mapperAction->getValue();
			if (targ != nullptr) {
				text = targ->userName->getValue();
			}
		}

		if (text != "") {
			text = action + "\n" + text;
		}
		return text;
	}
}
