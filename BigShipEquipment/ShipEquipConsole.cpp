

#include "TowardsTheStars.h"
#include "ShipEquipConsole.h"


AShipEquipConsole::AShipEquipConsole(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	CanUse = false;
	ObjectName = "Console";
}


