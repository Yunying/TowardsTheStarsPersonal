// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "UnrealNetwork.h"
#include "Scanner.h"

#define TRACE_WEAPON ECC_GameTraceChannel1


AScanner::AScanner(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	ScanTime = 0.5f;
	//ScannerState = EScannerState::NORMAL;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	ScanDistance = 10000.0f;
	SelectedAsteroidHealth = -100.0f;
}

void AScanner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AScanner, ScannerState);
	DOREPLIFETIME(AScanner, SelectedAsteroid);
	DOREPLIFETIME(AScanner, SelectedAsteroidHealth);
	DOREPLIFETIME(AScanner, SelectedScannerObject);
	DOREPLIFETIME(AScanner, SelectedScannerObjectHealth);
}

void AScanner::PrimaryFunction()
{
	if (Role <= ROLE_Authority)
	{
		ServerPrimaryFunction();
	}
}

void AScanner::SecondaryFunction()
{
	if (CameraBoom){
		CameraBoom->TargetArmLength = ZoomDistance;
		if (Cast<APlayerController>(Controller) != NULL){
			Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(ZoomFOV);
		}
	}
}

void AScanner::SecondaryFunctionReleased()
{
	if (CameraBoom){
		CameraBoom->TargetArmLength = 0.0f;
		Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(NormalFOV);
	}
}

void AScanner::Use(){
	//TArray<AActor*> CollectedActors;
	DebugPrint(FString::Printf(TEXT("[Scanner] On Use")));
	FVector CamLoc;
	FRotator CamRot;

	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	const FVector StartTrace = CamLoc;
	const FVector Direction = CamRot.Vector();
	const FVector EndTrace = StartTrace + Direction * ScanDistance;

	FCollisionQueryParams TraceParams(FName(TEXT("ItemTrace")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	DrawDebugLine(this->GetWorld(), StartTrace, EndTrace, FColor::Black, true, 10000.f, 10.f);
	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, TRACE_WEAPON, TraceParams);
	AActor *hit = Hit.GetActor();
	if (hit)
	{
		AAsteroidBase* asteroid = Cast<AAsteroidBase>(hit);
		if (asteroid)
		{
			CurrentAsteroid = asteroid;
			CurrentScannerObject = NULL;
			StartScanning();
			DebugPrint(FString::Printf(TEXT("[Scanner]Got an asteroid")));
		}
		AScannerObject* scannerobj = Cast<AScannerObject>(hit);
		if (scannerobj)
		{
			CurrentAsteroid = NULL;
			CurrentScannerObject = scannerobj;
			StartScanning();
			DebugPrint(FString::Printf(TEXT("[Scanner]Got an scanner obj")));
		}
		else
		{
			DebugPrint(FString::Printf(TEXT("[Scanner] hit something weird")));
			DebugPrint(hit->GetName());
		}
	}
}

void AScanner::OnEnter(){
	ScannerState = EScannerState::NORMAL;
}

void AScanner::StartScanning(){
	GetWorldTimerManager().SetTimer(this, &AScanner::StopScanning, ScanTime, false);
	ScannerState = EScannerState::SCANNING;
	/*APlayerController* controller = Cast<APlayerController>(Controller);
	AHUD* Myhud = controller->GetHUD();
	if (Myhud)
	{
		APlayerHUD* playerHUD = Cast<APlayerHUD>(Myhud);
		playerHUD->SetScanning(true);
	}
	else{
		DebugPrint(FString::Printf(TEXT("HUD is null")));
	}*/
	//DebugPrint(FString::Printf(TEXT("Scanner: Start Scanning")));
}

void AScanner::StopScanning(){
	ScannerState = EScannerState::NORMAL;
	if (CurrentAsteroid != NULL)
	{
		ScannedAsteroids.Add(CurrentAsteroid);
	}
	else if (CurrentScannerObject != NULL)
	{
		CurrentScannerObject->Scanned();
	}
	/*APlayerController* controller = Cast<APlayerController>(Controller);
	AHUD* hud = controller->GetHUD();
	if (hud)
	{
		APlayerHUD* playerHUD = Cast<APlayerHUD>(hud);
		playerHUD->SetScanning(false);
	}*/
	//DebugPrint(FString::Printf(TEXT("Scanner: Stop Scanning")));
}

void AScanner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}