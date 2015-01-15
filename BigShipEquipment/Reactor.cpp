// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "Reactor.h"


AReactor::AReactor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Energy_Functional = 80.0f;
	Energy_Damaged = 30.0f;
	Energy_Broken = 0.0f;

	ObjectName = "Reactor";
	ObjDescription = "Give you energy!";
}

void AReactor::BeginPlay()
{
	if (Role == ROLE_Authority)
	{
		AEquipmentManager::Instance()->addEquipment(this, REACTOR, false);
	}
}

void AReactor::CheckState(){
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
				if (!Sys){
					Sys = AShipSystem::GetShipSystem();
				}
				Sys->ReactorStateChanged();
				
			}
		}
		else if (CurrentHealth > DamagedCutOff)
		{
			if (EquipmentState != EEquipmentState::DAMAGED){
				EquipmentState = EEquipmentState::DAMAGED;
				if (!Sys){
					Sys = AShipSystem::GetShipSystem();
				}
				Sys->ReactorStateChanged();
			}
		}
		else
		{
			if (EquipmentState != EEquipmentState::BROKEN){
				EquipmentState = EEquipmentState::BROKEN;
				if (!Sys){
					Sys = AShipSystem::GetShipSystem();
				}
				Sys->ReactorStateChanged();
			}
		}
	}
}