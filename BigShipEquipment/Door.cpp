

#include "TowardsTheStars.h"
#include "Door.h"
#include "UnrealNetwork.h"


ADoor::ADoor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Door = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("DoorMesh"));
	Door->AttachTo(RootComponent);
	DoorLeft = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("DoorLeftMesh"));
	DoorLeft->AttachTo(RootComponent);
	Open = false;
	ObjectName = "Door";
	ObjDescription = "Are you leaving so soon?";
	DoorSound = nullptr;
	MovingAudioComponent = nullptr;
}

void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, Open);
}

void ADoor::SetOpen()
{
	if (Open)
	{
		OpenDoor();
		if (DoorSound)
			MovingAudioComponent = UGameplayStatics::PlaySoundAttached(DoorSound, RootComponent);

	}
	else
	{
		CloseDoor();
		if (DoorSound)
			MovingAudioComponent = UGameplayStatics::PlaySoundAttached(DoorSound, RootComponent);

	}
}

void ADoor::OpenDoor_Implementation()
{

}

void ADoor::CloseDoor_Implementation()
{

}
