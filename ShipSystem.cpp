// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "UnrealNetwork.h"
#include "ShipEngine.h"
#include "ShipEquipNPC.h"
#include "ShipLight.h"
#include "Reactor.h"
#include "ShieldGenerator.h"
#include "ShipSystem.h"
#include "RoomManager.h"
#include "TowardsTheStarsCharacter.h"
#include "TowardsTheStarsGameMode.h"

/*Global Pointer*/
static AShipSystem* ShipSysInstance = NULL;

AShipSystem::AShipSystem(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	MyNativeRootComponent = PCIP.CreateDefaultSubobject<USceneComponent>(this, "DefaultSceneRoot");
	SetRootComponent(MyNativeRootComponent);
	
	MaxHull = 100.0f;
	CurrentHull = 100.0f;
	LevelDistance = 1000.0f;
	Damage = 10.0f;
	HealthRegenPerSec = 0.2f;
	DeadPlayers = 0;
	IniEnergy = 3000.0f;
	EnergyRegenRate = 0;
	TestTimer = 0;
	ConstantEnergyUsage = 0;
	Debugging = false;
	UpdateTime = false;
}

AShipSystem* AShipSystem::GetShipSystem(){
	return ShipSysInstance;
}

void AShipSystem::OnInstantiation(AShipSystem* s){
	ShipSysInstance = s;
	IniEnergy = 3000.0f;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipEngine::StaticClass(), Engines);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShipShield::StaticClass(), Shields);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReactor::StaticClass(), Generators);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShieldGenerator::StaticClass(), ShieldGenerators);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomManager::StaticClass(), RoomManagers);
	for (auto Itr(Shields.CreateIterator()); Itr; Itr++)
	{
		AShipShield* e = Cast<AShipShield>(*Itr);
		Shield = e;
	}
	DebugPrint(FString::Printf(TEXT("[Ship System]Number of engines in the ship: %f"), Engines.Num()));

	DebugPrint(FString::Printf(TEXT("[Ship System] Number of Reactors in the ship: %f"), Generators.Num()));
	if (!Shield){
		DebugPrint(FString::Printf(TEXT("[Ship System] Missing Shield")));
		
	}
	for (auto Itr(Generators.CreateIterator()); Itr; Itr++)
	{
		AReactor* e = Cast<AReactor>(*Itr);
		EnergyRegenRate += e->Energy_Functional;
	}
	for (auto Itr(Engines.CreateIterator()); Itr; Itr++)
	{
		AShipEngine* e = Cast<AShipEngine>(*Itr);
		ConstantEnergyUsage += e->EnergyUsage;
	}
	for (auto Itr(ShieldGenerators.CreateIterator()); Itr; Itr++)
	{
		AShieldGenerator* e = Cast<AShieldGenerator>(*Itr);
		ConstantEnergyUsage += e->EnergyUsage;
	}

	//Added by Byron
	for (auto Itr(RoomManagers.CreateIterator()); Itr; Itr++)
	{
		ARoomManager* e = Cast<ARoomManager>(*Itr);
		e->Setup();
		e->FastOxygenRefreshRate = e->OxygenRefreshRate * 5;
	}
}


void AShipSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShipSystem, CurrentHull);
	DOREPLIFETIME(AShipSystem, IniEnergy);
	DOREPLIFETIME(AShipSystem, CurrentDistance);
	DOREPLIFETIME(AShipSystem, Shield);
	DOREPLIFETIME(AShipSystem, DeadPlayers);
}

void AShipSystem::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	CheatTick(DeltaSeconds);

	if (!UpdateTime)
		return;

	if (Debugging){
		TestTimer += DeltaSeconds;
		if (TestTimer > 3.0f){
			//DebugPrint(FString::Printf(TEXT("Current Energy: %f"), IniEnergy));
			TestTimer = 0.0f;
		}
	}
	/*if (Shield){
		DebugPrint(FString::Printf(TEXT("Has Shield")));
	}*/
	if (Role == ROLE_Authority){
		Regen(DeltaSeconds);
		GetDistance(DeltaSeconds);
		MonitorEnergy(DeltaSeconds);
		if (CurrentDistance >= LevelDistance){
			for (auto Itr(Engines.CreateIterator()); Itr; Itr++)
			{
				AShipEngine* e = Cast<AShipEngine>(*Itr);
				e->SetFTL(true); //[EndGameCondition]
				ATowardsTheStarsGameMode* gm = (ATowardsTheStarsGameMode*)GetWorld()->GetAuthGameMode();
				if (gm){
					gm->OnVictory();
				}

			}
		}
		//Added by Byron
		DeadPlayers = 0;
		for (auto Itr(RoomManagers.CreateIterator()); Itr; Itr++)
		{
			ARoomManager* e = Cast<ARoomManager>(*Itr);
			e->tickCount++;
			for (auto it = e->AdjacentRoomDoorMap.begin(); it != e->AdjacentRoomDoorMap.end(); it++){
				if (it->second->OxygenLevel - e->OxygenLevel > 1 || it->second->OxygenLevel - e->OxygenLevel < -1){
					if (it->first->Open){
						e->DoOxygen(it->second, e->FastOxygenRefreshRate*DeltaSeconds);
						//  old way of calculating oxygen level
						//	if ((e->tickCount%e->FastOxygenRefreshRate) == 0){
						//		e->DoOxygen(it->second);
						//		DebugPrint(FString::Printf(TEXT("%s: Refresh, new oxygen level %3.0f"), *(e->RoomName), e->OxygenLevel));
						//	}
						//}
						//else {
						//	if ((e->tickCount%e->OxygenRefreshRate) == 0){
						//		e->DoOxygen(it->second);
						//		DebugPrint(FString::Printf(TEXT("%s: Refresh, new oxygen level %3.0f"), *(e->RoomName), e->OxygenLevel));
						//	}
					}
					else {
						e->DoOxygen(it->second, e->OxygenRefreshRate*DeltaSeconds);
					}
				}
			}
			for (int i = 0; i < e->NoDoorRoomList.Num(); i++){
				if (e->NoDoorRoomList[i]->OxygenLevel - e->OxygenLevel > 1 || e->NoDoorRoomList[i]->OxygenLevel - e->OxygenLevel < -1){
					e->DoOxygen(e->NoDoorRoomList[i], e->FastOxygenRefreshRate*DeltaSeconds);
					DebugPrint(FString::Printf(TEXT("%s: Refresh, new oxygen level %3.0f"), *(e->RoomName), e->OxygenLevel));
				}
			}
		}
		for (int i = 0; i < PlayerList.Num(); i++)
		{
			ATowardsTheStarsCharacter* character = Cast<ATowardsTheStarsCharacter>(PlayerList[i]);
			if (character)
			{
				if (character->GetIsDead())
				{
					DeadPlayers++;
					DebugPrint("Player has died");
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Player has died")));
				}
			}
		}
		if (DeadPlayers >= PlayerList.Num() && PlayerList.Num() > 0){
			//Add Game Over code here
			DebugPrint("All players dead, Game Over");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("All players dead, Game Over")));
			ATowardsTheStarsGameMode* gm = (ATowardsTheStarsGameMode*)GetWorld()->GetAuthGameMode();
			if (gm){
				gm->OnDeath();
			}
		}
		//End of Byron's addition to code
	}
}

void AShipSystem::GetDistance(float DeltaSeconds){
	for (auto Itr(Engines.CreateIterator()); Itr; Itr++)
	{
		AShipEngine* e = Cast<AShipEngine>(*Itr);
		CurrentDistance += e->CurrentVelocity*DeltaSeconds;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Distance %f"), CurrentDistance));
	}
}

//void AShipSystem::OnAsteroidHit(){
//	if (Role == ROLE_Authority){
//		DebugPrint(FString::Printf(TEXT("Current Shiled: %f, Current Hull: %f"), ShieldCurrentHealth, CurrentHull));
//		ApplyDamage(Damage);
//	}
//}

void AShipSystem::Regen(float DeltaSeconds){
	//IniEnergy += EnergyRegenRate*DeltaSeconds;
	if (EnergyRegenRate == 0){
		for (auto Itr(Generators.CreateIterator()); Itr; Itr++)
		{
			AReactor* e = Cast<AReactor>(*Itr);
			EnergyRegenRate += e->Energy_Functional;
		}
		if (Debugging){
			DebugPrint(FString::Printf(TEXT("Energy Regen Rate: %f"), EnergyRegenRate));
			
		}
		
	}

	IniEnergy += EnergyRegenRate * DeltaSeconds;
	/*if (Shield){
		if (Shield->HealthRegen()){
			if (CurrentHull != MaxHull){
				CurrentHull += EnergyRegenRate*DeltaSeconds;
			}
		}
	}*/
	if (CurrentHull != MaxHull){
//		CurrentHull += HealthRegenPerSec*DeltaSeconds;
	}
	if (CurrentHull > MaxHull){
		CurrentHull = MaxHull;
	}
}

void AShipSystem::ApplyDamage(float inDamage){
	if (Shield){
		CurrentHull -= Shield->ApplyDamage(inDamage);
	}
	else{
		CurrentHull -= inDamage;
	}
	if (CurrentHull <= 0){
		//Ending game?
		CurrentHull = 0;
		DebugPrint(FString::Printf(TEXT("[ShipSystem] Hull has reached 0. Game Over")));
		ATowardsTheStarsGameMode* gm = (ATowardsTheStarsGameMode*)GetWorld()->GetAuthGameMode();
		if (gm){
			gm->OnDeath();
		}

	}
}

void AShipSystem::UseEnergy(float u){
	DebugPrint(FString::Printf(TEXT("Used Energy: %f"), u));
	IniEnergy -= u;
}

void AShipSystem::MonitorEnergy(float DeltaSeconds){
	IniEnergy -= ConstantEnergyUsage * DeltaSeconds;
	if (IniEnergy < 0){
		IniEnergy = 0;
	}
	/*for (auto Itr(Equips.CreateIterator()); Itr; Itr++)
	{
		if ((*Itr)->IsA<AShipEngine>()){
			AShipEngine* e = Cast<AShipEngine>(*Itr);
			IniEnergy -= e->EnergyUsage*DeltaSeconds;
		}
		else if ((*Itr)->IsA<AShipLight>()){
			AShipLight* e = Cast<AShipLight>(*Itr);
			IniEnergy -= e->EnergyUsage*DeltaSeconds;
		}

	}*/
}

float AShipSystem::GetEnergy(){
	return IniEnergy;
}

/*
void AShipSystem::SetMinimap(AMinimap* m){
	DebugPrint(FString::Printf(TEXT("Set Minimap")));
	Minimap = m;
	if (!Minimap){
		DebugPrint(FString::Printf(TEXT("Still No Map in system")));
	}
}
*/

void AShipSystem::ReactorStateChanged(){
	EnergyRegenRate = 0;
	for (auto Itr(Generators.CreateIterator()); Itr; Itr++)
	{
		AReactor* e = Cast<AReactor>(*Itr);
		//EnergyRegenRate += e->Energy_Functional;
		switch (e->EquipmentState){
			case(EEquipmentState::FUNCTIONAL):
				EnergyRegenRate += e->Energy_Functional;
				break;

			case(EEquipmentState::DAMAGED) :
				EnergyRegenRate += e->Energy_Damaged;
				break;

			case(EEquipmentState::BROKEN) :
				EnergyRegenRate += e->Energy_Broken;
				break;
			
		}
	}
}
#pragma region Cheats
void AShipSystem::CheatTick(float DeltaSeconds)
{
	if (bInfiniteShipHull)
	{
		//ShipSys = AShipSystem::GetShipSystem();
		this->CurrentHull = 100;
	}
	if (bInfiniteShipShield)
	{
		//ShipSys = AShipSystem::GetShipSystem();
		this->Shield->ShieldCurrentHealth = this->Shield->ShieldMaxHealth;
	}
	if (bInfiniteShipEnergy)
	{
		//ShipSys = AShipSystem::GetShipSystem();
		this->IniEnergy = 10000;
	}
}


#pragma endregion Cheats

bool AShipSystem::AddPlayer_Validate(ATowardsTheStarsCharacter* character)
{
	return true;
}

void AShipSystem::AddPlayer_Implementation(ATowardsTheStarsCharacter* character)
{
	if (Role == ROLE_Authority)
	{
		if (PlayerList.Find(character) == INDEX_NONE)
		{
			PlayerList.Push(character);
		}
	}
}