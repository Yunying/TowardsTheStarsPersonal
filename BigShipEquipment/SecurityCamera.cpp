// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "SecurityCamera.h"


ASecurityCamera::ASecurityCamera(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{

}

void ASecurityCamera::BeginPlay(){
	CameraBeginPlay();
	/*if (SceneCapture){
		SceneCapture->TextureTarget = SceneCaptureTexture;
		SceneCapture->Deactivate();
	}
	else{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("SecurityCamera: Add SceneCapture Component")));
	}*/
}

void ASecurityCamera::ActivateCamera_Implementation(bool a){

}

void ASecurityCamera::CameraBeginPlay_Implementation(){

}


