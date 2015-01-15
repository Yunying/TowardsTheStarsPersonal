

#include "TowardsTheStars.h"
#include "ElevatorConsole.h"


AElevatorConsole::AElevatorConsole(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Called = false;
	ObjectName = "Elevator Console";
	ObjDescription = "Call the elevator";
}

void AElevatorConsole::msgElevatorArrived(){
	DebugPrint(FString::Printf(TEXT("[Elevator] Elevator Arrived")));
	Called = false;
}

