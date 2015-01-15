// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "ShieldGenerator.h"


AShieldGenerator::AShieldGenerator(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Shield_Functional = 2.0f;
	Shield_Damaged = 1.0f;
	Shield_Broken = 0.5f;
	EnergyUsage = 20.0f;
	Disabled = false;

	ObjectName = "Shield Generator";
	ObjDescription = "Restore your shield!";
}

void AShieldGenerator::BeginPlay(){
	if (MyShield){
		MyShield->AddGenerator(this);
	}
	else{
		DebugPrint(FString::Printf(TEXT("[Shield Generator] Missing Shield System")));
	}
	if (Role == ROLE_Authority)
	{
		AEquipmentManager::Instance()->addEquipment(this, SHIELDGENERATOR, false);
	}
}

void AShieldGenerator::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	if (AShipSystem::GetShipSystem()->GetEnergy() < EnergyUsage && !Disabled){
		Disabled = true;
		if (MyShield){
			MyShield->GeneratorStateChanged();
		}
	}
	else if (AShipSystem::GetShipSystem()->GetEnergy() > EnergyUsage && Disabled){
		Disabled = false;
		if (MyShield){
			MyShield->GeneratorStateChanged();
		}
	}
}

void AShieldGenerator::CheckState(){
	if (Role == ROLE_Authority)
	{
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
		}

		if (CurrentHealth > FunctionalCutOff)
		{
			if (EquipmentState != EEquipmentState::FUNCTIONAL){
				EquipmentState = EEquipmentState::FUNCTIONAL;
				if (MyShield){
					MyShield->GeneratorStateChanged();
				}
				if (AShipSystem::GetShipSystem()->Debugging){
					DebugPrint(FString::Printf(TEXT("[Shield Generator] Shield Generator State Changed")));
				}
				
			}
		}
		else if (CurrentHealth > DamagedCutOff)
		{
			if (EquipmentState != EEquipmentState::DAMAGED){
				EquipmentState = EEquipmentState::DAMAGED;
				if (MyShield){
					MyShield->GeneratorStateChanged();
				}
				if (AShipSystem::GetShipSystem()->Debugging){
					DebugPrint(FString::Printf(TEXT("[Shield Generator] Shield Generator State Changed")));
				}
			}
		}
		else
		{
			if (EquipmentState != EEquipmentState::BROKEN){
				EquipmentState = EEquipmentState::BROKEN;
				if (MyShield){
					MyShield->GeneratorStateChanged();
				}
				if (AShipSystem::GetShipSystem()->Debugging){
					DebugPrint(FString::Printf(TEXT("[Shield Generator] Shield Generator State Changed")));
				}
			}
		}
	}
}