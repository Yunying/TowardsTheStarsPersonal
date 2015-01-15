

#include "TowardsTheStars.h"
#include "Turret.h"
#include "AsteroidBase.h"
#include "UnrealNetwork.h"

#define TRACE_WEAPON ECC_GameTraceChannel1

ATurret::ATurret(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	MaxAmmo = 4;
	OverChargeAmmo = 2;
	currentAmmo = MaxAmmo;
	rechargeTime = 3.f;

	OverCharged = false;

	timeSinceLastRecharge = 0;
	ParticleLocation1 = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc1"));
	ParticleLocation1->AttachTo(RootComponent);
	ParticleLocation2 = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc2"));
	ParticleLocation2->AttachTo(RootComponent);

	BaseTurnRate = 15.f;
	BaseLookUpRate = 15.f;
	ObjectName = "Turret";

	ZoomDistance = 5000.0f;
	ZoomFOV = 10;
	NormalFOV = 90;
}

void ATurret::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurret, currentAmmo);

}//*/

void ATurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Role == ROLE_Authority)
	{
		Recharge(DeltaSeconds);
	}

}

void ATurret::Recharge(float DeltaSeconds)
{
	timeSinceLastRecharge += DeltaSeconds;
	if (timeSinceLastRecharge>=rechargeTime)
	{
		timeSinceLastRecharge = 0;
		Reload();
	}
}

void ATurret::Reload()
{
	if (OverCharged)
	{
		if (currentAmmo < MaxAmmo + OverChargeAmmo)
		{
			currentAmmo += 1;
		}
	}
	else
	{
		if (currentAmmo < MaxAmmo)
		{
			currentAmmo += 1;
		}
	}
}

void ATurret::OverCharge()
{
	OverCharged = true;
}

void ATurret::PrimaryFunction()
{
	if (Role <= ROLE_Authority)
	{
		ServerPrimaryFunction();
	}
}

void ATurret::SecondaryFunction()
{
	isZoomed = true;
	CameraBoom->TargetArmLength = ZoomDistance;
	Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(ZoomFOV);
}

void ATurret::SecondaryFunctionReleased()
{
	isZoomed = false;
	CameraBoom->TargetArmLength = 0.0f;
	Cast<APlayerController>(Controller)->PlayerCameraManager->SetFOV(NormalFOV);
}

void ATurret::Use()
{
	if (currentAmmo > 0)
	{
		currentAmmo--;
		ShootEffect();
		//Do RayCast Stuff Here;
		TArray<AActor*> CollectedActors;
		FVector CamLoc;
		FRotator CamRot;

		Controller->GetPlayerViewPoint(CamLoc, CamRot);
		const FVector StartTrace = CamLoc;
		const FVector Direction = CamRot.Vector();
		const FVector EndTrace = StartTrace + Direction * 1000000;

		if (laserFire)
			MovingAudioComponent = UGameplayStatics::PlaySoundAttached(laserFire, RootComponent);

		FCollisionQueryParams TraceParams(FName(TEXT("ItemTrace")), true, this);
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;
		TraceParams.bTraceComplex = true;
		FHitResult Hit(ForceInit);
		GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, TRACE_WEAPON, TraceParams);
		AActor *hit = Hit.GetActor();

		AAsteroidBase* asteroid = Cast<AAsteroidBase> (hit);
		if(asteroid)
		{
			asteroid->TakeDamage(100.0f, FDamageEvent(), GetInstigatorController(), this);
		}
	}
}

bool ATurret::ShootEffect_Validate()
{
	return true;
}

void ATurret::ShootEffect_Implementation()
{
	ShootPC1 = UGameplayStatics::SpawnEmitterAttached(ShootFX, ParticleLocation1);
	ShootPC2 = UGameplayStatics::SpawnEmitterAttached(ShootFX, ParticleLocation2);
}

void ATurret::AmmoChange()
{		

	APlayerController* controller = Cast<APlayerController>(Controller);
	if (controller && controller->IsLocalPlayerController())
	{
		if (Cast<APlayerHUD>(controller->GetHUD()) != NULL)
		{
			Cast<APlayerHUD>(controller->GetHUD())->TurretAmmo = currentAmmo;
		}
		else{
			AmmoChangeHUD();
		}
		
	}
}

void ATurret::AmmoChangeHUD_Implementation(){

}