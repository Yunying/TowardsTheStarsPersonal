// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "SecurityMonitor.h"


ASecurityMonitor::ASecurityMonitor(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	IsOn = false;
	CurrentIndex = 0;
	Damaged = false;
}

void ASecurityMonitor::BeginPlay(){
	if (Cameras.Num() > 0){
		CurrentCamera = Cameras[0];
		CurrentIndex = 0;
		CurrentCamera->ActivateCamera(true);
		ShowCameraScene();
	}
}

void ASecurityMonitor::SwitchCamera(){
	//if (!IsOn){
	//	IsOn = true;
	//	if (Cameras.Num() > 0){
	//		CurrentCamera = Cameras[0];
	//		CurrentIndex = 0;
	//		CurrentCamera->ActivateCamera(true);
	//	}
	//	else{
	//		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("No Camera Assigned to this monitor")));
	//	}
	//}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Number of camera: %f"), Cameras.Num()));
	if (CurrentCamera){
		CurrentCamera->ActivateCamera(false);
		bool found = false;
		for (auto Itr(Cameras.CreateIterator()); Itr; Itr++)
		{

			if (found){
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Here")));
				ASecurityCamera* c = *Itr;
				if (c->EquipmentState == EEquipmentState::FUNCTIONAL){
					CurrentCamera = (*Itr);
					CurrentCamera->ActivateCamera(true);
					ShowCameraScene();
					found = false;
					break;
				}
			}
			if ((*Itr) == CurrentCamera){
				found = true;
			}
		}
		if (found){
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Back to first camera")));
			if (Cameras[0]->EquipmentState == EEquipmentState::FUNCTIONAL){
				CurrentCamera = Cameras[0];
				CurrentCamera->ActivateCamera(true);
				ShowCameraScene();
			}
		}
	}
	else{
		for (auto Itr(Cameras.CreateIterator()); Itr; Itr++)
		{
			if ((*Itr)->EquipmentState == EEquipmentState::FUNCTIONAL){
				CurrentCamera = *Itr;
				CurrentCamera->ActivateCamera(true);
				ShowCameraScene();
				break;
			}
		}
	}
}

void ASecurityMonitor::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
		if (CurrentCamera){
			if (CurrentCamera->EquipmentState != EEquipmentState::FUNCTIONAL){
				SwitchCamera();
			}
		}
}

void ASecurityMonitor::ShowCameraScene_Implementation(){

}

void ASecurityMonitor::CheckState(){
	if (Role == ROLE_Authority)
	{
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
			if (CurrentHealth > FunctionalCutOff)
			{
				EquipmentState = EEquipmentState::FUNCTIONAL;
				if (Damaged){
					CurrentCamera = Cameras[0];
					CurrentCamera->ActivateCamera(true);
					Damaged = false;
				}
			}
			else if (CurrentHealth > DamagedCutOff)
			{
				Damaged = true;
				EquipmentState = EEquipmentState::DAMAGED;
				if (CurrentCamera){
					CurrentCamera->ActivateCamera(false);
					CurrentCamera = NULL;
				}
			}
			else
			{
				Damaged = true;
				EquipmentState = EEquipmentState::BROKEN;
				if (CurrentCamera){
					CurrentCamera->ActivateCamera(false);
					CurrentCamera = NULL;
				}
			}
		}
	}
}


