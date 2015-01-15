

#include "TowardsTheStars.h"
#include "CanisterDispenser.h"
#include "LaserGun.h"
#include "HealingSpray.h"
#include "FireExtinguisher.h"
#include "RepairTool.h"
#include "Item.h"


ACanisterDispenser::ACanisterDispenser(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	FillRate = 0.1f;
	Frequency = 1.0f;
	FillAmount = 0.0f;
	Time = 0.0f;
	Maximum_Canister = 3;
	Current_Canister = 0;
	Damaged_Rate = 0.05f;
	Broken_Rate = 0.01f;
	AddHealth = 50.0f;
	AddAmmo = 5.0f;
	EnergyUsage = 100.0f;

	ObjectName = "Canister Dispencer";
}

void ACanisterDispenser::Tick(float DeltaSeconds){
	if (Role == ROLE_Authority)
	{
		Super::Tick(DeltaSeconds);
		Time += DeltaSeconds;
		if (Time > Frequency){
			Time = 0.0f;
			if (FillAmount >= 1.0f){
				FillAmount = 0;
				Dispense();
				DebugPrint(FString::Printf(TEXT("[Canister Dispenser] Dispense Amount: %f"), FillAmount));
			}
			else{
				Fill();
			}
		}
	}
}

void ACanisterDispenser::Fill(){
	if (Current_Canister < Maximum_Canister){
		switch (EquipmentState){
		case (EEquipmentState::FUNCTIONAL) :
			FillAmount += FillRate;
			break;

		case (EEquipmentState::DAMAGED) :
			FillAmount += Damaged_Rate;
			break;

		case (EEquipmentState::BROKEN) :
			FillAmount += Broken_Rate;
			break;
		}
		
	}
}

void ACanisterDispenser::Dispense(){
	if (HealthPackSpawn != NULL)
	{
		UWorld* const World = GetWorld();
		if (World)
		{
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			FRotator SpawnRotation;
			SpawnRotation.Yaw = FMath::FRand() * 360.f;
			SpawnRotation.Pitch = FMath::FRand() * 360.f;
			SpawnRotation.Roll = FMath::FRand() * 360.f;

			AItem* const SpawnedItem = World->SpawnActor<AItem>(HealthPackSpawn, SpawnPoint, SpawnRotation, SpawnParams);
			SpawnedItem->ParentDispenser = this;
			Current_Canister++;
			if (!Sys){
				Sys = AShipSystem::GetShipSystem();
			}
			Sys->UseEnergy(EnergyUsage);
		}
	}
	
}

void ACanisterDispenser::PickedUp()
{
	if (Role == ROLE_Authority)
	{
		DebugPrint(FString::Printf(TEXT("[Canister Dispenser] Canister PickedUp")));

		Current_Canister--;
	}
}

void ACanisterDispenser::OnUse(ATowardsTheStarsCharacter* c){
	APlayerEquipment* Equip = c->CurrentEquipment;
		switch (DispenserType){
		case (EDispenserCategory::ENERGY) :
			if (Equip){
				if (Equip->IsA<ALaserGun>()){
					DebugPrint(FString::Printf(TEXT("[Canister Dispenser] Use Dispenser")));
					Equip->IncreaseAmmo(AddAmmo);
					Sys->UseEnergy(EnergyUsage);
				}
			}
			break;

		case (EDispenserCategory::HEALTH) :
			//c->ApplyDamage(0 - AddHealth);
			//Sys->UseEnergy(EnergyUsage);
			if (Equip){
				if (Equip->IsA<AHealingSpray>()){// || Equip->IsA<ARepairTool>()){
					DebugPrint(FString::Printf(TEXT("[CanisterDispenser] Use Dispenser")));
					Equip->IncreaseAmmo(AddAmmo);
					Sys->UseEnergy(EnergyUsage);
				}
			}
			break;

		case (EDispenserCategory::WATER) :
			if (Equip){
				if (Equip->IsA<AFireExtinguisher>()){
					Equip->IncreaseAmmo(AddAmmo);
					Sys->UseEnergy(EnergyUsage);
				}
			}
			break;
		}
}

