

#include "TowardsTheStars.h"
#include "RapidTurret.h"
#include "AsteroidBase.h"
#include "UnrealNetwork.h"

#define TRACE_WEAPON ECC_GameTraceChannel1


ARapidTurret::ARapidTurret(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MaxChargeLevel = 4;
	OverChargeLevel = 5;
	currentChargeLevel = MaxChargeLevel;
	MinChargeLevel = 3.f;

	OverCharged = false;

	ParticleLocation1 = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc1"));
	ParticleLocation1->AttachTo(RootComponent);
	ParticleLocation2 = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc2"));
	ParticleLocation2->AttachTo(RootComponent);

	BaseTurnRate = 15.f;
	BaseLookUpRate = 15.f;
	ObjectName = "Rapid Turret";
	state = GunState::NotShooting;

	FireRate = .1f;

	ZoomDistance = 5000.0f;
	ZoomFOV = 10;
	NormalFOV = 90;
}

void ARapidTurret::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARapidTurret, currentChargeLevel);

}//*/

void ARapidTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Role == ROLE_Authority)
	{
		if (state != GunState::Shooting)
		{
			Recharge(DeltaSeconds);
		}
	}

}

void ARapidTurret::Shoot()
{
	currentChargeLevel -= .1;
	if (currentChargeLevel <= 0.f)
	{
		PrimaryFunctionReleased();
	}
	else //still shooting. perform Raycast;
	{
		//Do RayCast Stuff Here;
		TArray<AActor*> CollectedActors;
		FVector CamLoc;
		FRotator CamRot;

		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		const FVector StartTrace = CamLoc;
		const FVector Direction = CamRot.Vector();
		//1000000=10km
		const FVector EndTrace = StartTrace + Direction * 1000000;

		FCollisionQueryParams TraceParams(FName(TEXT("ItemTrace")), true, this);
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bTraceComplex = true;
		FHitResult Hit(ForceInit);
		GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, TRACE_WEAPON, TraceParams);
		AActor *hit = Hit.GetActor();

		AAsteroidBase* asteroid = Cast<AAsteroidBase>(hit);
		if (asteroid)
		{
			asteroid->TakeDamage(100.0f, FDamageEvent(), GetInstigatorController(), this);
		}
	}
}

void ARapidTurret::Recharge(float DeltaSeconds)
{
	if (OverCharged)
	{
		currentChargeLevel += DeltaSeconds;
		if (currentChargeLevel >= OverChargeLevel)
		{
			currentChargeLevel = OverChargeLevel;
		}
	}
	else
	{
		currentChargeLevel += DeltaSeconds;
		if (currentChargeLevel >= MaxChargeLevel)
		{
			currentChargeLevel = MaxChargeLevel;
		}
	}
}

void ARapidTurret::OverCharge()
{
	OverCharged = true;
}

void ARapidTurret::PrimaryFunction()
{
	if (Role <= ROLE_Authority)
	{
		ServerPrimaryFunction();
	}
}

void ARapidTurret::SecondaryFunction()
{
	isZoomed = true;
	CameraBoom->TargetArmLength = ZoomDistance;
	Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(ZoomFOV);
}

void ARapidTurret::SecondaryFunctionReleased()
{
	isZoomed = false;
	CameraBoom->TargetArmLength = 0.0f;
	Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(NormalFOV);
}

void ARapidTurret::Use()
{
	if (currentChargeLevel >= MinChargeLevel)
	{
		state = Shooting;
		GetWorldTimerManager().SetTimer(this, &ARapidTurret::Shoot, FireRate, true);
		ShootEffect();
	}
}

bool ARapidTurret::ShootEffect_Validate()
{
	return true;
}

void ARapidTurret::ShootEffect_Implementation()
{
	ShootPC1 = UGameplayStatics::SpawnEmitterAttached(ShootFX, ParticleLocation1);
	ShootPC2 = UGameplayStatics::SpawnEmitterAttached(ShootFX, ParticleLocation2);
	FireAc = PlayWeaponSound(FireLoopSound);
}

bool ARapidTurret::EndShootEffect_Validate()
{
	return true;
}

void ARapidTurret::EndShootEffect_Implementation()
{
	PlayWeaponSound(FireFinishSound);
	if (FireAc)
	{
		FireAc->Stop();
	}
	if (ShootPC1)
	{
		ShootPC1->DeactivateSystem();
	}
	if (ShootPC2)
	{
		ShootPC2->DeactivateSystem();
	}
}

void ARapidTurret::AmmoChange()
{

	APlayerController* controller = Cast<APlayerController>(Controller);
	if (controller && controller->IsLocalPlayerController())
	{
		if (Cast<APlayerHUD>(controller->GetHUD()) != NULL)
		{
			Cast<APlayerHUD>(controller->GetHUD())->TurretCharge = currentChargeLevel;
		}
		else{
			AmmoChangeHUD();
		}
	}
}

void ARapidTurret::AmmoChangeHUD_Implementation(){

}

void ARapidTurret::Stop()
{
	if (state == GunState::Shooting)
	{
		state = NotShooting;
		GetWorldTimerManager().ClearTimer(this, &ARapidTurret::Shoot);
		EndShootEffect();
	}
}

UAudioComponent* ARapidTurret::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound)
	{
		AC = UGameplayStatics::PlaySoundAttached(Sound, RootComponent);
	}

	return AC;
}
