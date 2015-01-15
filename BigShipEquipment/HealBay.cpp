

#include "TowardsTheStars.h"
#include "UnrealNetwork.h"
#include "HealBay.h"


AHealBay::AHealBay(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	ObjectName = "Heal Bay";
	AddedHealthAmount = 0.1f;
	AddedHealthAmount_Damaged = 0.05f;
	Frequency = 2.0f;
	ElapsedTime = 0;
	EnergyUsage = 20.0f;
	fHealRate = 20.0f;
	fRadius = 300.0f;

	if (Role != ROLE_Authority)
	{
		static ConstructorHelpers::FObjectFinder<UParticleSystem>HealObj(TEXT("ParticleSystem'/Game/Particles/healing_mist/P_Healing_Spray.P_Healing_Spray'"));//set the healing spray to spawn
		ClientHealFX = HealObj.Object;
	}
}

void AHealBay::PostInitializeComponents()
{
	Super::PostInitializeComponents(); // Remember to always call the parent implementation or else the game will crash
	TScriptDelegate<FWeakObjectPtr> OnBeginOverlapDelegate;
	OnBeginOverlapDelegate.BindUFunction(this, TEXT("OnTriggerVolumeBeginOverlap"));
	TScriptDelegate<FWeakObjectPtr> OnEndOverlapDelegate;
	OnEndOverlapDelegate.BindUFunction(this, TEXT("OnTriggerVolumeEndOverlap"));

	TriggerVolume->OnComponentBeginOverlap.Add(OnBeginOverlapDelegate);
	TriggerVolume->OnComponentEndOverlap.Add(OnEndOverlapDelegate);
}

void AHealBay::OnTriggerVolumeBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Role == ROLE_Authority)
	{
		ATowardsTheStarsCharacter* OverlappedCharacter = Cast<ATowardsTheStarsCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			if (OverlappedCharacters.Num() == 0)
			{
				OverlappedCharacters.Add(OverlappedCharacter);
			}
			else if (!OverlappedCharacters.Contains(OverlappedCharacter))
			{
				OverlappedCharacters.Add(OverlappedCharacter);
			}
		}
	}
	else//if client, do this
	{
		ATowardsTheStarsCharacter* OverlappedCharacter = Cast<ATowardsTheStarsCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			if (OverlappedCharacters.Num() == 0)
			{
				if (ClientHealFX)
				{
					if (EquipmentState != EEquipmentState::BROKEN)
					{
						ClientHealPC = UGameplayStatics::SpawnEmitterAttached(ClientHealFX, ParticleLocation);
					}
				}
				OverlappedCharacters.Add(OverlappedCharacter);
			}
			else if (!OverlappedCharacters.Contains(OverlappedCharacter))
			{
				OverlappedCharacters.Add(OverlappedCharacter);
			}
		}
	}
}

void AHealBay::OnTriggerVolumeEndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Role == ROLE_Authority)
	{
		ATowardsTheStarsCharacter* LeavingCharacter = Cast<ATowardsTheStarsCharacter>(OtherActor);
		if (LeavingCharacter)
		{
			if (OverlappedCharacters.Contains(LeavingCharacter))
			{
				OverlappedCharacters.Remove(LeavingCharacter);
			}
		}
	}
	else
	{
		ATowardsTheStarsCharacter* LeavingCharacter = Cast<ATowardsTheStarsCharacter>(OtherActor);
		if (LeavingCharacter)
		{
			if (OverlappedCharacters.Contains(LeavingCharacter))
			{
				OverlappedCharacters.Remove(LeavingCharacter);

				if (OverlappedCharacters.Num() == 0)
				{
					if (ClientHealPC)
					{
						// THis is where we turn it off
						ClientHealPC->Deactivate();
					}
				}
			}
		}
	}
}

void AHealBay::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//for each actor, try to heal them
	if (Role == ROLE_Authority)
	{
		ElapsedTime += DeltaSeconds;

		if (ElapsedTime > Frequency)
		{
			ElapsedTime = 0.f;
			if (OverlappedCharacters.Num() > 0)
			{
				for (int i = 0; i < OverlappedCharacters.Num(); i++)
				{
					if (EquipmentState == EEquipmentState::FUNCTIONAL)
					{
						OverlappedCharacters[i]->ApplyDamage(-1 * OverlappedCharacters[i]->mTrait->mBaseMaxHP*AddedHealthAmount);//apply negative value so that it heals
					}
					else if (EquipmentState == EEquipmentState::DAMAGED)
					{
						OverlappedCharacters[i]->ApplyDamage(-1 * OverlappedCharacters[i]->mTrait->mBaseMaxHP*AddedHealthAmount_Damaged);//apply negative value so that it heals
					}
				}
			}
		}
	}
}

void AHealBay::SetHealth()
{
	Super::SetHealth();

	if (Role != ROLE_Authority)
	{
		if (EquipmentState != EEquipmentState::BROKEN)
		{
			if (!ClientHealPC)
			{
				if (OverlappedCharacters.Num() > 0)
				{
					ClientHealPC = UGameplayStatics::SpawnEmitterAttached(ClientHealFX, ParticleLocation);
				}
			}
		}
	}
}
