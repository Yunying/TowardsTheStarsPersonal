// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "UnrealNetwork.h"
#include "ShieldGenerator.h"
#include "ShipShield.h"


AShipShield::AShipShield(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bReplicates = true;

	ShieldMaxHealth = 20.0f;
	//ShieldCurrentHealth = 20.0f;
	RegenRatePerSec = 0;
	ShieldsDown = false;
	RechargeDelay = 5.0f;
	RechargeTime = 0.0f;

	TriggerVolume = PCIP.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("TriggerVolume"));
	TriggerVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	RootComponent = TriggerVolume;

	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	ObjectName = "Ship Shield";
	ObjDescription = "Getting you places safe!";
}

void AShipShield::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShipShield, ShieldCurrentHealth);
}

void AShipShield::BeginPlay(){
	ShieldCurrentHealth = ShieldMaxHealth;
}

//Apply damage to shield first, rest to ship HP
float AShipShield::ApplyDamage(float inDamage){
	if (ShieldCurrentHealth >= inDamage){
		ShieldCurrentHealth -= inDamage;
		return 0;
	}
	else if (ShieldCurrentHealth > 0){
		inDamage -= ShieldCurrentHealth;
		ShieldCurrentHealth = 0;
		ShieldsDown = true;
		return inDamage;
	}
	else{
		ShieldCurrentHealth = 0;
		ShieldsDown = true;
		return inDamage;
	}
}

void AShipShield::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	if (Role == ROLE_Authority){
		if (!ShieldsDown)
		{
			HealthRegen(DeltaSeconds);
		}
		else
		{
			RechargeTime += DeltaSeconds;
			ShieldsDown = true;
			if (RechargeTime >= RechargeDelay)
			{
				RechargeTime = 0.0f;
				ShieldsDown = false;
			}
		}
	}
}

bool AShipShield::HealthRegen(float DeltaSeconds){
	if (RegenRatePerSec == 0){
		GeneratorStateChanged();
	}
	if (ShieldCurrentHealth < ShieldMaxHealth){
		ShieldCurrentHealth += RegenRatePerSec*DeltaSeconds;
		if (ShieldCurrentHealth > ShieldMaxHealth){
			ShieldCurrentHealth = ShieldMaxHealth;
		}
		return false;
	}
	else{
		return true;
	}
}

void AShipShield::AddGenerator(AActor* a){
	AShieldGenerator* sg = Cast<AShieldGenerator>(a);
	Generators.Add(sg);
	GeneratorStateChanged();
}

void AShipShield::GeneratorStateChanged(){
	RegenRatePerSec = 0;
	for (auto Itr(Generators.CreateIterator()); Itr; Itr++)
	{
		AShieldGenerator* e = Cast<AShieldGenerator>(*Itr);
		//EnergyRegenRate += e->Energy_Functional;
		if (!e->Disabled){
			switch (e->EquipmentState){
			case(EEquipmentState::FUNCTIONAL) :
				RegenRatePerSec += e->Shield_Functional;
				break;

			case(EEquipmentState::DAMAGED) :
				RegenRatePerSec += e->Shield_Damaged;
				break;

			case(EEquipmentState::BROKEN) :
				RegenRatePerSec += e->Shield_Broken;
				break;
			}
		}
	}
	if (AShipSystem::GetShipSystem()->Debugging){
		DebugPrint(FString::Printf(TEXT("[ShipSystem]Current Shield Regen Rate: %f"), RegenRatePerSec));
	}
	
}


