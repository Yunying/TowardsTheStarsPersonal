
#include "TowardsTheStars.h"
#include "ShipEquipNPC.h"
#include "UnrealNetwork.h"


AShipEquipNPC::AShipEquipNPC(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bReplicates = true;
	MaxHealth = 100.f;
	FunctionalCutOff = 75.f;
	DamagedCutOff = 40.f;
	RegenRate = 0.f;
	CurrentHealth = MaxHealth;
	EquipmentState = EEquipmentState::FUNCTIONAL;
	
	ObjMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ObjectMesh"));
	SetRootComponent(ObjMesh);
	ObjMesh->SetNetAddressable();
	ObjMesh->SetIsReplicated(true);

	TriggerVolume = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("TriggerVolume"));
	TriggerVolume->AttachTo(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;
	ObjectName = "ShipEquipNPC";
	ObjDescription = " ";


	//Visual Stuff
	ParticleLocation = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc"));
	ParticleLocation->AttachTo(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem>BrokenObj(TEXT("ParticleSystem'/Game/Particles/P_Explosion.P_Explosion'"));
	BrokenFX = BrokenObj.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem>DamageObj(TEXT("ParticleSystem'/Game/Particles/P_FireExt_Smoke.P_FireExt_Smoke'"));
	DamageFX = DamageObj.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem>HealObj(TEXT("ParticleSystem'/Game/Particles/P_Sparks.P_Sparks'"));
	HealFX = HealObj.Object;

	PingMenuJson = FString::Printf(TEXT("constructPingMenu(\"[{ \\\"id\\\": 1, \\\"name\\\": \\\"Repair\\\", \\\"functionString\\\": \\\"Repair\\\" },{ \\\"id\\\": 2, \\\"name\\\": \\\"Assist\\\", \\\"Assist\\\": \\\"function2\\\" },{ \\\"id\\\": 3, \\\"name\\\": \\\"Using\\\", \\\"functionString\\\": \\\"Using\\\" }]\")"));
}

void AShipEquipNPC::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(this, &AShipEquipNPC::GetSys, 1.0f, false);
}

void AShipEquipNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShipEquipNPC, CurrentHealth);
}

void AShipEquipNPC::ApplyDamage(float inDamage)
{
	if (Role == ROLE_Authority)
	{
		if (CurrentHealth < inDamage){
			CurrentHealth = 0;
		}
		else{
			CurrentHealth -= inDamage;
		}
		CheckState();
		/*if (CurrentHealth > FunctionalCutOff)
		{
			EquipmentState = EEquipmentState::FUNCTIONAL;
		}
		else if (CurrentHealth > DamagedCutOff)
		{
			EquipmentState = EEquipmentState::DAMAGED;
		}
		else
		{
			EquipmentState = EEquipmentState::BROKEN;
		}*/
	}
}

void AShipEquipNPC::AddHealth(float inHealth)
{
	if (Role == ROLE_Authority)
	{
		CurrentHealth += inHealth;
		CheckState();
		/*if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
			if (CurrentHealth > FunctionalCutOff)
			{
				EquipmentState = EEquipmentState::FUNCTIONAL;
			}
			else if (CurrentHealth > DamagedCutOff)
			{
				EquipmentState = EEquipmentState::DAMAGED;
			}
			else
			{
				EquipmentState = EEquipmentState::BROKEN;
			}
		}*/
	}
}

void AShipEquipNPC::UseEnergy()
{
}

void AShipEquipNPC::SetHealth()
{
	if (HealthPC != NULL)
	{
		HealthPC->Deactivate();
	}
	if (CurrentHealth > FunctionalCutOff)
	{
		EquipmentState = EEquipmentState::FUNCTIONAL;
		if (CurrentHealth != MaxHealth){
			HealthPC = UGameplayStatics::SpawnEmitterAttached(HealFX, ParticleLocation);
		}
	}
	else if (CurrentHealth > DamagedCutOff)
	{
		EquipmentState = EEquipmentState::DAMAGED;
		//particle/ visual effect here
		HealthPC = UGameplayStatics::SpawnEmitterAttached(DamageFX, ParticleLocation);
	}
	else
	{
		EquipmentState = EEquipmentState::BROKEN;
		//particle/ visual effect here
		HealthPC = UGameplayStatics::SpawnEmitterAttached(BrokenFX, ParticleLocation);
	}
}

void AShipEquipNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Role == ROLE_Authority)
	{
		AddHealth(DeltaSeconds * RegenRate);
	}
}

void AShipEquipNPC::ActorInteract_Implementation(ATowardsTheStarsCharacter* inCharacter)
{

}
void AShipEquipNPC::GetSys(){
	Sys = AShipSystem::GetShipSystem();
}

void AShipEquipNPC::CheckState(){
	if (Role == ROLE_Authority)
	{
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
			if (CurrentHealth > FunctionalCutOff)
			{
				EquipmentState = EEquipmentState::FUNCTIONAL;
			}
			else if (CurrentHealth > DamagedCutOff)
			{
				EquipmentState = EEquipmentState::DAMAGED;
			}
			else
			{
				EquipmentState = EEquipmentState::BROKEN;
			}
		}
	}
}

void AShipEquipNPC::Destroyed()
{
	if (Role == ROLE_Authority)
	{
		if (AEquipmentManager::Instance()){
			AEquipmentManager::Instance()->removeEquipment(this);
		}
	}
	return Super::Destroyed();
}