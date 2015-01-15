

#include "TowardsTheStars.h"
#include "TowardsTheStarsCharacter.h"
#include "CoffeeMaker.h"

ACoffeeMaker::ACoffeeMaker(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	/** Setup default data */
	EnergyUsage = 0.0f;
	SpeedBoost = 1000.0f;
	BoostDuration = 30.0f;
	SpeedReduction = -400.0f;
	ObjectName = "Coffee Maker";
	ObjDescription = "Drink Coffee for temporary Speed Boost";
}


void ACoffeeMaker::UseCoffeeMaker(ATowardsTheStarsCharacter* character, float Speed){
	Sys->UseEnergy(EnergyUsage);

	if(Role == ROLE_Authority)
	{
		character->mTrait->AddTraitEvent(BoostDuration, SpeedBoost, COFFEE_MAKER, MOVEMENT_SPEED, REPLACE);
	}

}
