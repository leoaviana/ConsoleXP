#pragma once
#include "Game.h"


namespace Highlight
{ 
	/*
	Will apply a highlight effect exactly like what is applied on mouseover a unit/object...
	If desired spell visual effects also can be applied by setting the parameter enabledAura to true. that will apply
	a spell base effect into the closest unit to simulate a highlight effect. this will only work with Units, GameObjects will only get standard highlighting.
	The default spell ID stands for the spell PS Engineering - Arena Aura Gold
	*/

	extern uint64_t lastCandidateInteract; // last candidate highlighted, used to clear the highlight when no candidates are found
	extern uint64_t lastCandidateMouseOver; // last candidate highlighted, used to clear the highlight when no candidates are found

	uint32_t HighlightInteract(bool enableAura = false, uint32_t spellId = 54273);
	uint32_t HighlightMouseOver(bool enableAura = false, uint32_t spellId = 54273);
}