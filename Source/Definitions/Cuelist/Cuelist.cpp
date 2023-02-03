/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Cuelist.h"
#include "Brain.h"
#include "../ChannelValue.h"
#include "CuelistManager.h"
#include "UI/CuelistLoadWindow.h"
#include "UI/GridView/CuelistGridView.h"
#include "UI/ConductorInfos.h"
#include "UserInputManager.h"

int sortCues(Cue* A, Cue* B) {
	String test = A->id->getValue() > B->id->getValue() ? "y" : "n";
	return A->id->getValue() > B->id->getValue() ? 1 : -1;
}

Cuelist::Cuelist(var params) :
	BaseItem(params.getProperty("name", "Cuelist")),
	objectType(params.getProperty("type", "Cuelist").toString()),
	objectData(params),
	conductorInfos("Conductor infos"),
	chaserOptions("Chaser options"),
	offFadeCurve(),
	cues()
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	editorIsCollapsed = true;
	itemDataType = "Cuelist";
	canBeDisabled = false;

	id = addIntParameter("ID", "Id of this Cuelist", params.getProperty("ID", 1), 1);
	userName = addStringParameter("Name", "Name of this cuelist", "New cuelist");
	layerId = addIntParameter("Layer", "Higher layer, higer priority", 1, 1);
	updateName();

	currentCueName = conductorInfos.addStringParameter("Current cue", "Current Cue name", "", false);
	currentCueText = conductorInfos.addStringParameter("Current cue text", "What's happening during this cue ?", "", false);
	currentCueText->multiline = true;
	nextCueGo = conductorInfos.addStringParameter("Next go", "action needed to go to next cue", "", false);
	nextCueName = conductorInfos.addStringParameter("Next Cue", "Next cue name", "", false);
	addChildControllableContainer(&conductorInfos);

	isChaser = chaserOptions.addBoolParameter("Is Chaser", "Turn this cuelist in a wonderful chaser", false);
	chaserDirection = chaserOptions.addEnumParameter("Direction", "");
	chaserDirection->addOption("Normal", "normal");
	chaserDirection->addOption("Reverse", "reverse");
	chaserDirection->addOption("Bounce", "bounce");
	chaserDirection->addOption("Random", "random");
	chaserSpeed = chaserOptions.addFloatParameter("Speed", "in GO / minutes", 60,0.001);
	chaserInFade = chaserOptions.addFloatParameter("In fade", "Fade for incoming steps, not in seconds, but in number of steps !",0,0,1);
	chaserOutFade = chaserOptions.addFloatParameter("Out fade", "Fade for out values, not in seconds, but in number of steps !", 0, 0);
	chaserRunXTimes = chaserOptions.addFloatParameter("Run X times", "number of cycles of the chaser, 0 mean infinite",0,0);
	chaserStepPerTap = chaserOptions.addFloatParameter("Step per hits", "Number of steps per tap tempo hit",1,0.001);
	chaserTapTempo = chaserOptions.addTrigger("Tap tempo", "");

	chaserFadeInCurve.allowKeysOutside = false;
	chaserFadeInCurve.setNiceName("In curve");
	chaserFadeInCurve.isSelectable = false;
	chaserFadeInCurve.length->setValue(1);
	chaserFadeInCurve.addKey(0, 0, false);
	chaserFadeInCurve.items[0]->easingType->setValueWithData(Easing::LINEAR);
	chaserFadeInCurve.addKey(1, 1, false);
	chaserFadeInCurve.selectItemWhenCreated = false;
	chaserFadeInCurve.editorCanBeCollapsed = true;
	chaserFadeInCurve.editorIsCollapsed = true;
	chaserOptions.addChildControllableContainer(&chaserFadeInCurve);

	chaserFadeOutCurve.allowKeysOutside = false;
	chaserFadeOutCurve.setNiceName("Out curve");
	chaserFadeOutCurve.isSelectable = false;
	chaserFadeOutCurve.length->setValue(1);
	chaserFadeOutCurve.addKey(0, 0, false);
	chaserFadeOutCurve.items[0]->easingType->setValueWithData(Easing::LINEAR);
	chaserFadeOutCurve.addKey(1, 1, false);
	chaserFadeOutCurve.selectItemWhenCreated = false;
	chaserFadeOutCurve.editorCanBeCollapsed = true;
	chaserFadeOutCurve.editorIsCollapsed = true;
	chaserOptions.addChildControllableContainer(&chaserFadeOutCurve);

	chaserOptions.saveAndLoadRecursiveData = true;
	addChildControllableContainer(&chaserOptions);

	endAction = addEnumParameter("Loop", "Behaviour of this cuelist at the end of its cues");
	endAction->addOption("Off", "off");
	endAction->addOption("Loop", "loop");

	offIfOverwritten = addBoolParameter("Off If Overwritten", "If checked, this cuelist will kill itself if all parameter are overwritten by other cuelists", false);

	tracking = addEnumParameter("Tracking", "Tracking will keep the values of previous cues enabled");
	tracking->addOption("None", "none");
	tracking->addOption("Cuelist order", "cuelist");
	tracking->addOption("Call order", "call");

	loopTracking = addBoolParameter("Loop Tracking", "Do you want tracking keeps values when coming back to start after loop ?", false);

	autoStart = addBoolParameter("Auto start", "Starts the cuelist if HTP level is set to a different value than 0", false);
	autoStop = addBoolParameter("Auto stop", "Stops the cuelist if HTP level is set to 0", false);

	goBtn = addTrigger("GO", "Trigger next cue");
	goBackBtn = addTrigger("GO back", "Trigger previous cue");
	goRandomBtn = addTrigger("GO random", "Trigger a random cue");
	offBtn = addTrigger("OFF", "Off this cuelist");
	killBtn = addTrigger("KILL", "Kill this cuelist and leave no clues");
	loadBtn = addTrigger("Load", "Choose next cue");
	loadAndGoBtn = addTrigger("Load and go", "Choose a cue");
	loadRandomBtn = addTrigger("Load Random", "Load a random cue");
	// flashOnBtn = addTrigger("Flash ON", "release flash");
	// flashOffBtn = addTrigger("flash Off", "press flash");
	// swopOnBtn = addTrigger("Swop ON", "press swop");
	// swopOffBtn = addTrigger("Swop Off", "release swop");

	nextCue = addTargetParameter("Next Cue", "Cue triggered when button go pressed", &cues);
	nextCue->maxDefaultSearchLevel = 0;
	nextCue->targetType = TargetParameter::CONTAINER;

	nextCueId = addFloatParameter("Next cue ID", "ID of the cue triggered when go pressed, 0 means next cue",0,0);

	HTPLevel = addFloatParameter("HTP Level", "Level master for HTP channels of this sequence", 1, 0, 1);
	FlashLevel = addFloatParameter("Flash Level", "Flash/swop level master for HTP channels of this sequence", 1, 0, 1);
	LTPLevel = addFloatParameter("LTP Level", "Level master for LTP channels of this sequence", 1, 0, 1);

	isCuelistOn = addBoolParameter("is On", "Is this cuelist on ?", false);
	isCuelistOn->isControllableFeedbackOnly = true;

	currentFade = addFloatParameter("Current Xfade", "Current position of the running cue", 0, 0, 1);
	currentFade->isControllableFeedbackOnly = true;

	offFade = addFloatParameter("Off time", "Default fade time used to off the cuelist", 0, 0);

	// offFadeCurve = new Automation();
	offFadeCurve.saveAndLoadRecursiveData = true;
	offFadeCurve.setNiceName("Off curve");
	offFadeCurve.allowKeysOutside = false;
	offFadeCurve.isSelectable = false;
	offFadeCurve.length->setValue(1);
	offFadeCurve.addKey(0, 0, false);
	offFadeCurve.items[0]->easingType->setValueWithData(Easing::LINEAR);
	offFadeCurve.addKey(1, 1, false);
	offFadeCurve.selectItemWhenCreated = false;
	offFadeCurve.editorCanBeCollapsed = true;
	addChildControllableContainer(&offFadeCurve);

	//nextCueId = addFloatParameter("Next Cue", "Index of the next cue", - 1, -1);
	cueA = nullptr; // static current cue
	cueB = nullptr; // loaded Cue

	// currentCue = addTargetParameter("Current Cue", "Last cue triggered", cues.get());
	// currentCue->maxDefaultSearchLevel = 0;
	// currentCue->targetType = TargetParameter::CONTAINER;

	renumberCuesBtn = addTrigger("Renumber cues", "Reset all cues IDs");

	addChildControllableContainer(&cues);

	cues.comparator.compareFunc = &sortCues;
	reorderCues();

	fillTexts();

	if (params.isVoid()) {
		cues.addItem();
	}

	Brain::getInstance()->registerCuelist(this, id->getValue());
	if (!Brain::getInstance()->loadingIsRunning) {
		CuelistGridView::getInstance()->updateCells();
	}
}

Cuelist::~Cuelist()
{
	kill(true);
	Brain::getInstance()->unregisterCuelist(this);
}

void Cuelist::reorderCues() {
	cues.reorderItems();
	//cues->queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, cues.get()));
}

void Cuelist::onContainerParameterChangedInternal(Parameter* p) {
	if (Brain::getInstance()->loadingIsRunning) {
		//return;
	}
	if (p == HTPLevel || p == FlashLevel) {
		Brain::getInstance()->virtualFadersNeedUpdate = true;
		if (p == HTPLevel && !Brain::getInstance()->loadingIsRunning) {
			if (autoStart->getValue() && cueA == nullptr && (float)HTPLevel->getValue() != 0 && lastHTPLevel == 0) {
				go();
			}
			else if (autoStop->getValue() && cueA!=nullptr && (float)HTPLevel->getValue() == 0) {
				off();
			}
		}
		pleaseUpdateHTPs = true;
		lastHTPLevel = p->getValue();
		Brain::getInstance()->pleaseUpdate(this);
	}
	if (p == LTPLevel) {
		Brain::getInstance()->virtualFadersNeedUpdate = true;
		pleaseUpdateLTPs = true;
		Brain::getInstance()->pleaseUpdate(this);
	}

	if (p == id) {
		Brain::getInstance()->registerCuelist(this, id->getValue(), true);
	}
	if (p == userName || p == id) {
		updateName();
		CuelistGridView::getInstance()->updateCells();
	}
}

void Cuelist::triggerTriggered(Trigger* t) {
	if (t == goBtn) {
		go();
	}
	else if (t == goBackBtn) {
		goBack();
	}
	else if (t == goRandomBtn) {
		goRandom();
	}
	else if(t == offBtn) {
		off();
	}
	else if (t == killBtn) {
		kill();
	}
	else if (t == flashOnBtn) {
		flash(true, true);
	}
	else if (t == flashOffBtn) {
		flash(false, true);
	}
	else if (t == swopOnBtn) {
		flash(true, true, true);
	}
	else if (t == swopOffBtn) {
		flash(false, true);
	}
	else if (t == renumberCuesBtn) {
		renumberCues();
	}
	else if (t == loadBtn) {
		showLoad();
	}
	else if (t == loadAndGoBtn) {
		showLoadAndGo();
	}
	else if (t == loadRandomBtn) {
		loadRandom();
	}
	else {}
}

void Cuelist::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) {
	if (c == chaserSpeed) {
		chaserStepDuration = 60000.0/(double)chaserSpeed->getValue();
		chaserFadeInDuration = chaserStepDuration * (double)chaserInFade->getValue();
		chaserFadeOutDuration = chaserStepDuration * (double)chaserOutFade->getValue();;

	}
	else if (c == chaserInFade) {
		chaserFadeInDuration = chaserStepDuration * (double)chaserInFade->getValue();
	}
	else if (c == chaserOutFade) {
		chaserFadeOutDuration = chaserStepDuration * (double)chaserOutFade->getValue();;
	}
	else if (c == chaserTapTempo) {
		tapTempo();
	}
	
}


void Cuelist::go(Cue* c) {
	const MessageManagerLock mmLock;
	double now = Time::getMillisecondCounterHiRes();;
	if (cueA != nullptr) {
		cueA->TSAutoFollowEnd = 0;
	} 


	if ((float)chaserRunXTimes->getValue() > 0) {
		if (cueA == nullptr && isChaser->getValue()) {
			if (chaserDirection->getValueData() == "bounce" && cues.items.size()>1) {
				int nItems = cues.items.size();
				nItems = (nItems*2)-2;
				chaserRemainingSteps = round((float)chaserRunXTimes->getValue() * nItems);
			}
			else {
				chaserRemainingSteps = round((float)chaserRunXTimes->getValue() * cues.items.size());
				chaserRemainingSteps-=1;
			}
		}
		else if (chaserRemainingSteps <= 0 && !wannaOff) {
			wannaOff = true;
			c = nullptr;
		}
		else {
			chaserRemainingSteps -= 1;
		}

	}

	bool needRebuildTracking = false;
	String trackingType = tracking->getValue();

	int currentIndex = -1;
	int nextIndex = -1;
	if (c != nullptr && trackingType == "cuelist") {
		nextIndex = cues.items.indexOf(c);
		if (cueA != nullptr) {
			currentIndex = cues.items.indexOf(cueA);
		}

		if (nextIndex != currentIndex + 1) {
			needRebuildTracking = true;
		}
	}

	cueA = c;
	cueB = nullptr;
	nextCue->resetValue();
	nextCueId->resetValue();
	fillTexts();
	if (c != nullptr) {
		isCuelistOn->setValue(true);
		wannaOff = false;
	}
	else {
		wannaOff = true;
		Brain::getInstance()->pleaseUpdate(this);
	}

	HashMap<SubFixtureChannel*, ChannelValue*> newActiveValues;

	if (needRebuildTracking) {
		for (int i = 0; i <= nextIndex-1; i++) {
			Cue* tempCue = cues.items[i];
			tempCue->computeValues();
			for (auto it = tempCue->computedValues.begin(); it != tempCue->computedValues.end(); it.next()) {
				ChannelValue* temp = it.getValue();
				if (newActiveValues.contains(it.getKey())) {
					//ChannelValue* current = newActiveValues.getReference(it.getKey());
					temp->startValue = it.getKey()->postCuelistValue;
				}
				else {
					temp->startValue = -1;
				}
				temp->TSInit = now;
				temp->TSStart = now + (temp->delay);
				temp->TSEnd = temp->TSStart + (temp->fade);
				temp->isEnded = false;
				newActiveValues.set(it.getKey(), temp);
				it.getKey()->cuelistOnTopOfStack(this);
				Brain::getInstance()->pleaseUpdate(it.getKey());
			}
			tempCue->go();
		}
	}

	if (c != nullptr) {
		c->computeValues();
		for (auto it = c->computedValues.begin(); it != c->computedValues.end(); it.next()) {
			ChannelValue* temp = it.getValue();
			if (activeValues.contains(it.getKey())) {
				ChannelValue * current = activeValues.getReference(it.getKey());
				if (it.getKey()->isHTP) {
					if (current != nullptr) {
						double fader = HTPLevel->getValue();
						temp->startValue = current->endValue*fader;
					}
					else {
						temp->startValue = 0;
					}
				}
				else {
					temp->startValue = it.getKey()->postCuelistValue;
				}
			}
			else {
				temp->startValue = -1;
			}
			if (isChaser->getValue()) {
				temp->TSInit = now;
				temp->TSStart = now;
				temp->TSEnd = temp->TSStart + chaserFadeInDuration;
				temp->fadeCurve = &chaserFadeInCurve;
			}
			else {
				temp->TSInit = now;
				temp->TSStart = now + (temp->delay);
				temp->TSEnd = temp->TSStart + (temp->fade);
			}
			temp->isEnded = false;
			newActiveValues.set(it.getKey(), temp);
			it.getKey()->cuelistOnTopOfStack(this);
			Brain::getInstance()->pleaseUpdate(it.getKey());
		}
		c->go();
	}
	
	if (trackingType == "none" || c == nullptr || needRebuildTracking || isChaser->getValue()) {
		for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
			if (!newActiveValues.contains(it.getKey())) {
				ChannelValue* temp = it.getValue();
				if (temp != nullptr && temp -> endValue != -1) {
					float fadeTime = 0;
					float delay = 0;
					if (isChaser->getValue()) {
						fadeTime = chaserFadeOutDuration;
						temp->fadeCurve = &chaserFadeOutCurve;
					}
					else if (c == nullptr) {
						fadeTime = (float)offFade->getValue() * 1000;
						temp->fadeCurve = &offFadeCurve;
					}
					else {
						if (it.getKey()->isHTP) {
							fadeTime = (float)c->htpOutFade->getValue() * 1000;
							delay = (float)c->htpOutDelay->getValue() * 1000;
						}
						else {
							fadeTime = (float)c->ltpFade->getValue() * 1000;
							delay = (float)c->ltpDelay->getValue() * 1000;
						}
					}

					temp->TSInit = now;
					temp->TSStart = now + delay;
					temp->TSEnd = now + fadeTime + delay;
					temp->endValue = -1;
					temp->startValue = temp->value;
					temp->isEnded = false;

					newActiveValues.set(it.getKey(), temp);
					Brain::getInstance()->pleaseUpdate(it.getKey());
				}

			}
		}
	}
	
	for (auto it = newActiveValues.begin(); it != newActiveValues.end(); it.next()) {
		activeValues.set(it.getKey(), it.getValue());
	}

	if (isChaser->getValue() && c != nullptr) {
		c->TSAutoFollowStart = now;
		c->TSAutoFollowEnd = now + (chaserStepDuration);
		Brain::getInstance()->pleaseUpdate(c);

	}
	return ;
}

void Cuelist::go() {
	cueB = dynamic_cast<Cue*>(nextCue->targetContainer.get());
	float nextId = nextCueId->getValue();
	if (nextId > 0) {
		for (int i = 0; i < cues.items.size() && cueB == nullptr; i++) {
			if ((float)cues.items[i]->id->getValue() >= nextId) {
				cueB = cues.items[i];
			}
		}
	}
	if (cueB == nullptr) {
		if (isChaser->getValue()) {
			cueB = getNextChaserCue();
		}
		else {
			autoLoadCueB();
		}
	}
	go(cueB);
}


void Cuelist::goBack() {
	if (cueA == nullptr) {
		return;
	}
	Array<Cue*> currentCues = cues.getItemsWithType<Cue>();
	for (int i = 0; i < currentCues.size()-1; i++) {
		if (currentCues[i + 1] == cueA) {
			go(currentCues[i]);
			return;
		}
	}
}

void Cuelist::flash(bool setOn, bool withTiming, bool swop) {
	if (setOn) {
		isFlashing = true;
		double now = Time::getMillisecondCounterHiRes();

		Cue* flashingCue = cueA;
		if (flashingCue == nullptr) {
			autoLoadCueB();
			flashingCue = cueB;
		}

		cueB = nullptr;

		if (swop) {
			isSwopping = true;
			Brain::getInstance()->swoppedCuelist(this);
		}

		if (flashingCue != nullptr) {
			ScopedLock locK(Brain::getInstance()->usingCollections);
			flashingCue->computeValues();
			for (auto it = flashingCue->computedValues.begin(); it != flashingCue->computedValues.end(); it.next()) {
				ChannelValue* temp = it.getValue();
				if (flashingValues.contains(it.getKey())) {
					// ChannelValue* current = flashingValues.getReference(it.getKey());
					temp->startValue = it.getKey()->value;
				}
				else {
					temp->startValue = -1;
				}
				temp->TSInit = now;
				temp->TSStart = withTiming ? now + (temp->delay) : now;
				temp->TSEnd = withTiming ? temp->TSStart + (temp->fade) : now;
				temp->isEnded = false;
				flashingValues.set(it.getKey(), temp);
				it.getKey()->cuelistOnTopOfFlashStack(this);
				Brain::getInstance()->pleaseUpdate(it.getKey());
			}
		}
	}
	else {
		wannaOffFlash = true;
		isSwopping = false;
		Brain::getInstance()->unswoppedCuelist(this);
		for (auto it = flashingValues.begin(); it != flashingValues.end(); it.next()) {
			Brain::getInstance()->pleaseUpdate(it.getKey());
		}
		Brain::getInstance()->pleaseUpdate(this);
	}
}


void Cuelist::goRandom() {
	Array<Cue*> childCues = cues.getItemsWithType<Cue>();
	Array<Cue*> allowedCues;

	for (int i = 0; i< childCues.size(); i++) {
		if (childCues.getReference(i) !=cueA && childCues[i]->canBeRandomlyCalled->getValue()) {
			allowedCues.add(childCues.getReference(i));
		}
	}

	int s = allowedCues.size();
	if (s > 0) {
		int r = rand()%s;
		go(allowedCues[r]);
	}
}

void Cuelist::off() {
	wannaOff = true;
	go(nullptr);
}








void Cuelist::update() {
	if (pleaseUpdateHTPs) {
		updateHTPs();
	}
	if (pleaseUpdateLTPs) {
		updateLTPs();
	}
	float tempPosition = 1;
	float isUseFul = false;
	float isOverWritten = true;
	for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
		ChannelValue* cv = it.getValue();
		if (cv != nullptr) {
			tempPosition = jmin(tempPosition, cv->currentPosition);
			isUseFul = isUseFul || cv->endValue != -1 || !cv->isEnded;
			isOverWritten = isOverWritten && cv->isOverWritten;
		}
	}
	currentFade->setValue(tempPosition);
	if (tempPosition == 1 && cueA != nullptr) {
		cueA->endTransition();
	}
	if (isOverWritten) {
		isUseFul = false;
	}
	if (!isUseFul && offIfOverwritten->getValue()) {
		kill(false);
	} 

	if (wannaOffFlash) {
		bool canStopFlash = true;
		for (auto it = flashingValues.begin(); it != flashingValues.end(); it.next()) {
			ChannelValue* cv = it.getValue();
			canStopFlash = canStopFlash && cv->isEnded;
		}

		if (canStopFlash) {
			for (auto it = flashingValues.begin(); it != flashingValues.end(); it.next()) {
				it.getKey()->cuelistOutOfFlashStack(this);
				Brain::getInstance()->pleaseUpdate(it.getKey());
			}
			isFlashing = false;
			wannaOffFlash = false;
		}
	}
}

void Cuelist::autoLoadCueB() {
	bool valid = false;
	if (cueA == nullptr) {
		if (cues.getItemsWithType<Cue>().size() > 0) {
			cueB = cues.getItemsWithType<Cue>()[0];
			valid = true;
		}
	}
	else {
		Array<Cue*> currentCues = cues.getItemsWithType<Cue>();
		for (int i = 1; i < currentCues.size() && !valid; i++) {
			if (currentCues[i - 1] == cueA) {
				valid = true;
				cueB = currentCues[i];
			}
		}
		if (!valid) { // cueA is the last cue
			String end = endAction->getValue();
			if (end == "loop") {
				cueB = currentCues[0];
			}
			else {
				cueB = nullptr;
			}
		}
	}
	fillTexts();
}

float Cuelist::applyToChannel(SubFixtureChannel* fc, float currentVal, double now, bool flashvalues) {
	float val = currentVal;
	bool HTP = fc->parentParamDefinition->priority->getValue() == "HTP";
	
	bool keepUpdate = false;

	float localValue = 0;
	ChannelValue* cv;
	float faderLevel = 0;
	if (flashvalues) {
		if (!flashingValues.contains(fc)) { return currentVal; }
		cv = flashingValues.getReference(fc);
		faderLevel = (float)FlashLevel->getValue();
	}
	else {
		if (!activeValues.contains(fc)) { return currentVal; }
		cv = activeValues.getReference(fc);
		faderLevel = (float)HTPLevel->getValue();
	}

	if (cv == nullptr) {
		LOG("c'est chelou");
		return currentVal; 
	}

	float valueFrom = currentVal;
	float valueTo = currentVal;
	float outIsOff = false;
	// comportement OK, calcul de valeurs nok
	
	if (cv->startValue >= 0) {
		valueFrom = cv->startValue; 
	}
	if (cv->endValue >= 0) { 
		valueTo = cv->endValue; 
		if (HTP) {
			valueTo *= faderLevel;
		}
		else {
			valueTo *= (double)LTPLevel->getValue();
		}
	}
	else {
		outIsOff= true;
	}
	if (cv -> TSStart > now) {
		localValue = valueFrom; 
		keepUpdate = true;
	}
	else if (cv -> TSEnd <= now) {
		localValue = valueTo;
		cv->currentPosition = 1;
		if (!cv->isEnded) {
			cv->isEnded = true;
			Brain::getInstance()->pleaseUpdate(this);
		}
	}
	else {
		float totTime = (cv->delay + cv->fade);
		if (totTime > 0) {
			cv->currentPosition = (now - cv->TSInit) / (totTime);
		}
		else {
			cv->currentPosition = 1;
		}
		float fade = double(now - cv->TSStart) / double(cv -> TSEnd - cv->TSStart);
		fade = cv->fadeCurve->getValueAtPosition(fade);
		localValue = jmap(fade, valueFrom, valueTo);
		keepUpdate = true;
		Brain::getInstance()->pleaseUpdate(this);
	}
	cv->value = localValue;

	if (HTP) {
		val = jmax(val, localValue);
	}
	else {
		// float ratio = LTPLevel->getValue();
		// val = jmap(ratio,valueFrom,localValue);
		val = localValue;
	}

	if (keepUpdate) {
		Brain::getInstance()->pleaseUpdate(fc);
	}
	return val;
}

void Cuelist::cleanActiveValues() {
	Array<SubFixtureChannel*> temp;
	for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
		if (it.getValue()->value == -1) {
			temp.add(it.getKey());
		}
	}
	for (int i = 0; i < temp.size(); i++) {
		activeValues.remove(temp[i]);
	}
}

void Cuelist::insertProgCueBefore(Cue* c)
{
	int index = cues.items.indexOf(c);
	insertProgCueAtIndex(index);
}

void Cuelist::insertProgCueAfter(Cue* c)
{
	int index = cues.items.indexOf(c);
	insertProgCueAtIndex(index+1);
}

void Cuelist::insertProgCueAtIndex(int index)
{
	const MessageManagerLock mmlock;
	Programmer* p = UserInputManager::getInstance()->getProgrammer(false);
	if (p != nullptr) {
		Cue * cue = cues.addItem();
		cues.setItemIndex(cue, index);
		cue->commands.clear();
		for (int i = 0; i < p->commands.items.size(); i++) {
			Command* com = cue->commands.addItem();
			com->loadJSONData(p->commands.items[i]->getJSONData());
		}
	}

}

void Cuelist::updateHTPs() {
	pleaseUpdateHTPs = false;
	for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
		SubFixtureChannel* chan = it.getKey();
		if (chan!=nullptr && chan->isHTP) {
			Brain::getInstance()->pleaseUpdate(chan);
		}
	}
	for (auto it = flashingValues.begin(); it != flashingValues.end(); it.next()) {
		SubFixtureChannel* chan = it.getKey();
		if (chan != nullptr && chan->isHTP) {
			Brain::getInstance()->pleaseUpdate(chan);
		}
	}
}

void Cuelist::updateLTPs() {
	pleaseUpdateLTPs = false;
	for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
		SubFixtureChannel* chan = it.getKey();
		if (chan != nullptr && !chan->isHTP) {
			Brain::getInstance()->pleaseUpdate(chan);
		}
	}
}


void Cuelist::kill(bool forceRefreshChannels) {
	wannaOff = true;
	for (auto it = activeValues.begin(); it != activeValues.end(); it.next()) {
		SubFixtureChannel* chan = it.getKey();
		chan->cuelistOutOfStack(this);
		if (forceRefreshChannels) {
			Brain::getInstance()->pleaseUpdate(chan);
		}
	}
	cueA = nullptr;
	cueB = nullptr;
	isCuelistOn->setValue(false);
	fillTexts();
}

void Cuelist::setHTPLevel(float level) {
	HTPLevel->setValue(level);
}

void Cuelist::setFlashLevel(float level) {
	FlashLevel->setValue(level);
}

void Cuelist::setLTPLevel(float level) {
	LTPLevel->setValue(level);
}

void Cuelist::setChaserSpeed(float level) {
	chaserSpeed->setValue(level);
}

void Cuelist::showLoad()
{
	CuelistLoadWindow::getInstance()->loadCuelist(this, false);
}

void Cuelist::showLoadAndGo()
{
	CuelistLoadWindow::getInstance()->loadCuelist(this, true);
}


void Cuelist::updateName() {
	String n = userName->getValue();
	if (parentContainer != nullptr) {
		dynamic_cast<CuelistManager*>(parentContainer.get())->reorderItems();
	}
	setNiceName(String((int)id->getValue()) + " - " + n);
}

void Cuelist::renumberCues() {
	Array<Cue*> temp;
	temp.addArray(cues.items);
	for (int i = 0; i < temp.size(); i++) {
		temp[i]->id->setValue(i+1);
	}
}

void Cuelist::loadRandom() {
	Array<Cue*> childCues = cues.getItemsWithType<Cue>();
	Array<Cue*> allowedCues;

	for (int i = 0; i < childCues.size(); i++) {
		if (childCues.getReference(i) != cueA && childCues[i]->canBeRandomlyCalled->getValue()) {
			allowedCues.add(childCues.getReference(i));
		}
	}

	int s = allowedCues.size();
	if (s > 0) {
		int r = rand() % s;
		nextCue->setValueFromTarget(allowedCues[r]);
		fillTexts();
	}
}

void Cuelist::fillTexts() {
	if (cueA != nullptr) {
		currentCueName->setValue(cueA->niceName);
		currentCueText->setValue(cueA->cueText->getValue());
	}
	else {
		currentCueName->setValue("");
		currentCueText->setValue("");
	}
	Cue* n = getNextCue();
	if (n) {
		nextCueGo->setValue(n->goText->getValue());
		nextCueName->setValue(n->niceName);
	}
	else {
		nextCueGo->setValue("");
		nextCueName->setValue("");
	}
	if ((int)id->getValue() == 1) {
		ConductorInfos::getInstance()->repaint();
	}
}

Cue* Cuelist::getNextCue() {
	bool valid = false;
	if (cueA == nullptr) {
		if (cues.getItemsWithType<Cue>().size() > 0) {
			return cues.getItemsWithType<Cue>()[0];
		}
	}
	else {
		Array<Cue*> currentCues = cues.getItemsWithType<Cue>();
		for (int i = 1; i < currentCues.size() && !valid; i++) {
			if (currentCues[i - 1] == cueA) {
				return currentCues[i];
			}
		}
		String end = endAction->getValue();
		if (end == "loop") {
			return currentCues[0];
		}
	}
	return nullptr;
}

Cue* Cuelist::getNextChaserCue() {
	bool valid = false;
	if (cues.getItemsWithType<Cue>().size() == 0) {
		return nullptr;
	}
	if (cues.getItemsWithType<Cue>().size() == 1) {
		return cues.getItemsWithType<Cue>()[0];
	}

	if (chaserDirection->getValueData() == "random") {
		Array<Cue*> childCues = cues.getItemsWithType<Cue>();
		Array<Cue*> allowedCues;

		for (int i = 0; i < childCues.size(); i++) {
			if (childCues.getReference(i) != cueA) {
				allowedCues.add(childCues.getReference(i));
			}
		}

		int s = allowedCues.size();
		if (s > 0) {
			int r = rand() % s;
			return allowedCues[r];
		}
	}

	if (cueA == nullptr) {
		if ((chaserIsGoingBackward && chaserDirection->getValueData() == "bounce") || chaserDirection->getValueData() == "reverse") {
			return cues.getItemsWithType<Cue>()[cues.getItemsWithType<Cue>().size()-1];
		}
		else {
			return cues.getItemsWithType<Cue>()[0];
		}
	}
	else {
		Array<Cue*> currentCues = cues.getItemsWithType<Cue>();
		if ((chaserIsGoingBackward && chaserDirection->getValueData() == "bounce") || chaserDirection->getValueData() == "reverse") {
			for (int i = 1; i < currentCues.size() && !valid; i++) {
				if (currentCues[i] == cueA) {
					return currentCues[i-1];
				}
			}
		}
		else {
			for (int i = 1; i < currentCues.size() && !valid; i++) {
				if (currentCues[i - 1] == cueA) {
					return currentCues[i];
				}
			}
		}
		
		if (chaserDirection->getValueData() == "normal") {
			return cues.getItemsWithType<Cue>()[0];
		}
		else if (chaserDirection->getValueData() == "reverse") {
			return cues.getItemsWithType<Cue>()[cues.getItemsWithType<Cue>().size() - 1];
		}
		else if (chaserDirection->getValueData() == "bounce") {
			if (chaserIsGoingBackward) {
				chaserIsGoingBackward = false;
				return cues.getItemsWithType<Cue>()[1];
			}
			else {
				chaserIsGoingBackward = true;
				return cues.getItemsWithType<Cue>()[cues.getItemsWithType<Cue>().size() - 2];
			}
		}
		return nullptr;
	}

}

void Cuelist::tapTempo() {
	double now = Time::getMillisecondCounterHiRes();
	double delta = now - lastTapTempo;
	lastTapTempo = now;
	if (delta < 3000) {
		delta = delta / (double)chaserStepPerTap->getValue();
		double cpm = 60000. / delta;
		chaserSpeed->setValue(cpm);
	}
}