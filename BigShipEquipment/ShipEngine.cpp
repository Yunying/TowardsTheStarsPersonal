

#include "TowardsTheStars.h"
#include "AsteroidBase.h"
#include "ShipEngine.h"


AShipEngine::AShipEngine(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Power = 5000.0f;
	BaseVelocity = 1.0f;
	TotalUnits = 0.0f;
	//PowerDeduction = 1.0f;
	EnergyUsage = 100.0f;
	ObjectName = "Ship Engine";
	ObjDescription = "Getting you places fast!";
}

void AShipEngine::BeginPlay(){
	CurrentVelocity = BaseVelocity;
	FTL = false;
	if (Role == ROLE_Authority)
	{
		AEquipmentManager::Instance()->addEquipment(this, SHIPENGINE, false);
	}
	LevelDistance = AShipSystem::GetShipSystem()->LevelDistance;
}

void AShipEngine::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	if (Role == ROLE_Authority)
	{
		SetVelocity();
		TotalUnits += CurrentVelocity*DeltaSeconds;
	}
	CurrentDistance = AShipSystem::GetShipSystem()->CurrentDistance;
	
	//Power -= PowerDeduction;
}

void AShipEngine::SetVelocity(){
	if (AShipSystem::GetShipSystem()){
		if (AShipSystem::GetShipSystem()->GetEnergy() < EnergyUsage){
			CurrentVelocity = 0;
			DebugPrint(FString::Printf(TEXT("[Ship Engine] Not Enough Energy. Velocity is now 0")));
			return;
		}
	}
	else if (FTL){
		CurrentVelocity = 1.2*BaseVelocity;
		return;
	}
	else {
		switch (EquipmentState){
		case(EEquipmentState::FUNCTIONAL) :
			CurrentVelocity = BaseVelocity;
			break;

		case(EEquipmentState::BROKEN) :
			CurrentVelocity = 0.5 * BaseVelocity;
			break;

		case(EEquipmentState::DAMAGED) :
			CurrentVelocity = 0.2 * BaseVelocity;
			break;

		}
	}
}

void AShipEngine::SetFTL(bool inValue){
	FTL = inValue;
}
