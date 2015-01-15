

#include "TowardsTheStars.h"
#include "UnrealNetwork.h"
#include "ShipLight.h"


AShipLight::AShipLight(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Functional_Brightness = 100.0f;
	Damaged_Brightness = 30.0f;
	Broken_Brightness = 5.0f;
	EnergyUsage = 10.0f;

	bReplicates = true;

	PointLight1 = PCIP.CreateDefaultSubobject<UPointLightComponent>(this, "PointLight1");
	PointLight1 -> AttachTo(RootComponent);
	PointLight1->Intensity = Functional_Brightness;
	//PointLight1->bVisible = true;
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
}


void AShipLight::ToggleLight(){
	
	if (Role == ROLE_Authority){
		DebugPrint(FString::Printf(TEXT("[Ship Equipment Light] Toggle Light")));
		PointLight1->ToggleVisibility();
	}
}

void AShipLight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShipLight, PointLight1);
}
