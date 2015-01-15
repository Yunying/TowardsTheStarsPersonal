
#include "TowardsTheStars.h"
#include "ShipEquipPC.h"
#include "UnrealNetwork.h"
#include "AsteroidSpawnManager.h"
#include "TowardsTheStarsGameMode.h"


AShipEquipPC::AShipEquipPC(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bReplicates = true;
	//bAlwaysRelevant = true;
	MaxHealth = 100.f;
	FunctionalCutOff = 75.f;
	DamagedCutOff = 40.f;
	RegenRate = 0.f;
	CurrentHealth = MaxHealth;
	EquipmentState = EEquipmentState::FUNCTIONAL;
	isZoomed = false;

	ObjMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ObjectMesh"));
	SetRootComponent(ObjMesh);
	ObjMesh->SetNetAddressable();
	ObjMesh->SetIsReplicated(true);
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	TriggerVolume = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("TriggerVolume"));
	TriggerVolume->AttachTo(RootComponent);
	possessed = false;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	CharacterMovement->bAllowConcurrentTick = false;
	CharacterMovement->Activate(false);
	Mesh->Activate(false);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 0.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->AttachTo(RootComponent);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;


	MaxUpRotate = 45.f;
	MaxSideRotate = 45.f;
	//Visual Stuff
	ParticleLocation = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("ParticleLoc"));
	ParticleLocation->AttachTo(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem>BrokenObj(TEXT("ParticleSystem'/Game/Particles/P_Explosion.P_Explosion'"));
	BrokenFX = BrokenObj.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem>DamageObj(TEXT("ParticleSystem'/Game/Particles/P_FireExt_Smoke.P_FireExt_Smoke'"));
	DamageFX = DamageObj.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem>HealObj(TEXT("ParticleSystem'/Game/Particles/P_Sparks.P_Sparks'"));
	HealFX = HealObj.Object;

	ObjectName = "ShipEquipPC";

	ObjDescription = " ";

	PingMenuJson = FString::Printf(TEXT("constructPingMenu(\"[{ \\\"id\\\": 1, \\\"name\\\": \\\"Repair\\\", \\\"functionString\\\": \\\"Repair\\\" },{ \\\"id\\\": 2, \\\"name\\\": \\\"Assist\\\", \\\"Assist\\\": \\\"function2\\\" },{ \\\"id\\\": 3, \\\"name\\\": \\\"Using\\\", \\\"functionString\\\": \\\"Using\\\" }]\")"));
}

void AShipEquipPC::BeginPlay()
{
	Super::BeginPlay();
	StartRotation = GetActorRotation();

	if (Role == ROLE_Authority)
	{
		Cast<ATowardsTheStarsGameMode>(GetWorld()->GetAuthGameMode())->AnnouncementManager->RegisterListener(this, eAnnouncementLocation_All);
	}
}

void AShipEquipPC::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	
	InputComponent->BindAction("Interact", IE_Pressed, this, &AShipEquipPC::Exit);
	InputComponent->BindAction("Help", IE_Pressed, this, &AShipEquipPC::AsteroidControl);

	InputComponent->BindAxis("Turn", this, &AShipEquipPC::TurnAtRate);
	InputComponent->BindAxis("TurnRate", this, &AShipEquipPC::TurnAtRateController);
	InputComponent->BindAxis("LookUp", this, &AShipEquipPC::LookUpAtRate);
	InputComponent->BindAxis("LookUpRate", this, &AShipEquipPC::LookUpAtRateController);


	InputComponent->BindAction("PrimaryFunction", IE_Pressed, this, &AShipEquipPC::PrimaryFunction);
	InputComponent->BindAction("PrimaryFunction", IE_Released, this, &AShipEquipPC::PrimaryFunctionReleased);

	InputComponent->BindAction("SecondaryFunction", IE_Pressed, this, &AShipEquipPC::SecondaryFunction);
	InputComponent->BindAction("SecondaryFunction", IE_Released, this, &AShipEquipPC::SecondaryFunctionReleased);

}

void AShipEquipPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShipEquipPC, CurrentHealth);
}

void AShipEquipPC::ApplyDamage(float inDamage)
{
	if (Role == ROLE_Authority)
	{
		CurrentHealth -= inDamage;
	}
}

void AShipEquipPC::AddHealth(float inHealth)
{
	if (Role == ROLE_Authority)
	{
		CurrentHealth += inHealth;
		if (CurrentHealth > MaxHealth)
		{
			CurrentHealth = MaxHealth;
		}
	}
}

void AShipEquipPC::SetHealth()
{
	if (HealthPC != NULL)
	{
		HealthPC->Deactivate();
	}
	if (CurrentHealth > FunctionalCutOff)
	{
		EquipmentState = EEquipmentState::FUNCTIONAL;
		HealthPC = UGameplayStatics::SpawnEmitterAttached(HealFX, ParticleLocation);
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

void AShipEquipPC::Tick(float DeltaSeconds)
{
	//Super::Tick(DeltaSeconds);

	AddHealth(DeltaSeconds * RegenRate);
	if (needsToUpdateChat)
	{
		UpdateTextChatContent_Turret();
	}
}

void AShipEquipPC::ActorInteract_Implementation(ATowardsTheStarsCharacter* inCharacter)
{
	if (Role == ROLE_Authority)
	{
		if (!possessed)
		{
			APlayerController* controller = Cast<APlayerController>(inCharacter->GetController());
			
			
			//inCharacter->Mesh->SetVisibility(false);
			//inCharacter->Mesh->SetCollistionEnabled(ECollisionEnabled::NoCollision);
			//inCharacter->SetActorEnableCollision(false);
			//inCharacter->CurrentEquipment->SetActorHiddenInGame(true);
			//inCharacter->SetActorHiddenInGame(true);
			
			SetVisibility(inCharacter, false);

			inCharacter->DisableCharacter();
			inCharacter->UnPossessed();
			controller->Possess(this);
			
			possessed = true;
			oldCharacter = inCharacter; 
			ChangeHUD(false);
			CharacterMovement->Activate(true);
			controller->SetControlRotation(StartRotation);
			controller->ClientSetRotation(StartRotation);
		}
	}
	else
	{
		APlayerController* controller = Cast<APlayerController>(inCharacter->GetController());
		controller->SetControlRotation(StartRotation);
	}
	
}

void AShipEquipPC::Exit()
{
	if (Role <= ROLE_Authority)
	{
		ServerExit();
	}
}

bool AShipEquipPC::ServerExit_Validate()
{
	return true;
}

void AShipEquipPC::ServerExit_Implementation()
{
	SetVisibility(oldCharacter, true);
	oldCharacter->EnableCharacter();
	possessed = false;
	this->SetActorRotation(StartRotation);
	ChangeHUD(true);
	APlayerController* controller = Cast<APlayerController>(Controller);
	UnPossessed();
	controller->Possess(oldCharacter);
}

void AShipEquipPC::TurnAtRate(float Rate)
{
	TurnAtRateController(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShipEquipPC::TurnAtRateController(float Rate)
{
	// calculate delta for this frame from the rate information
	if (Rate != 0)
	{
		if (Role <= ROLE_Authority)
		{
			float AddedYaw = Rate;
			FRotator currentRot = GetActorRotation();

			if (Controller && Controller->IsLocalPlayerController())
			{
				APlayerController* const PC = CastChecked<APlayerController>(Controller);

				float currentYaw = (currentRot.Yaw - StartRotation.Yaw);
				currentYaw = fmodf(currentYaw, 360.f);

				if (currentYaw < 0)
				{
					currentYaw += 360.0f;
				}

				if (currentYaw > 180)
				{
					currentYaw = currentYaw - 360.f;
				}


				if ((currentYaw+ AddedYaw > MaxSideRotate) & (AddedYaw >0))
				{
					//printf("HERE");
				}
				else if ((currentYaw + AddedYaw < -1* MaxSideRotate) &( AddedYaw < 0))
				{
					
					//printf("HERE");
				}
				else
				{
					PC->AddYawInput(AddedYaw);
				}

			}
			ServerTurn(Rate);
		}
	}
}

bool AShipEquipPC::ServerTurn_Validate(float Rate)
{
	return true;
}

void AShipEquipPC::ServerTurn_Implementation(float Rate)
{
	//this->SetActorRotation();
}

void AShipEquipPC::LookUpAtRate(float Rate)
{
	LookUpAtRateController(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShipEquipPC::LookUpAtRateController(float Rate)
{
	if (Rate != 0)
	{
		if (Role <= ROLE_Authority)
		{
			float AddedPitch = -1 * Rate;
			FRotator currentRot = GetActorRotation();
			if (Controller && Controller->IsLocalPlayerController())
			{
				APlayerController* const PC = CastChecked<APlayerController>(Controller);
				float deltaPitch = (currentRot.Pitch - StartRotation.Pitch);
				deltaPitch = fmodf(deltaPitch , 360.f);

				if (deltaPitch < 0)
				{
					deltaPitch += 360.0f;
				}

				if (deltaPitch > 180)
				{
					deltaPitch = deltaPitch - 360.f;
				}


				if ( (deltaPitch+ AddedPitch > MaxUpRotate) & (AddedPitch <0))
				{
					//printf("HERE");
				}
				else if ((deltaPitch + AddedPitch < -1*MaxUpRotate) &(AddedPitch > 0))
				{
					//printf("HERE");
				}
				else
				{
					PC->AddPitchInput(AddedPitch);
				}
				ServerTurnUp(Rate);
			}
		}
	}
}

bool AShipEquipPC::ServerTurnUp_Validate(float Rate)
{
	return true;
}

void AShipEquipPC::ServerTurnUp_Implementation(float Rate)
{
	//this->SetActorRotation(GetControlRotation());
}

void AShipEquipPC::PrimaryFunction()
{
	//
}

void AShipEquipPC::SecondaryFunction()
{
	//
}

bool AShipEquipPC::ServerPrimaryFunction_Validate()
{
	return true;
}

void AShipEquipPC::ServerPrimaryFunction_Implementation()
{
	Use();
}

bool AShipEquipPC::ServerOnReleased_Validate()
{
	return true;
}

void AShipEquipPC::ServerOnReleased_Implementation()
{
	Stop();
}

void AShipEquipPC::Use()
{

}

bool AShipEquipPC::ChangeHUD_Validate(bool Exit)
{
	return true;
}

void AShipEquipPC::ChangeHUD_Implementation(bool Exit)
{
	
	APlayerController* controller = Cast<APlayerController>(Controller);
	AHUD* hud = controller->GetHUD();
	if (Cast<APlayerHUD>(hud) != NULL)
	{
		APlayerHUD* playerHUD = Cast<APlayerHUD>(hud);
		playerHUD->ChangeHUD(Exit);
	}
	else{
		ChangeHUDCast(Exit);
	}
	
	CharacterMovement->Activate(true);
}

void AShipEquipPC::ChangeHUDCast_Implementation(bool Exit){

}

void AShipEquipPC::PrimaryFunctionReleased()
{
	if (Role <= ROLE_Authority)
	{
		ServerOnReleased();
	}
}

void AShipEquipPC::SecondaryFunctionReleased()
{
	//
}

void AShipEquipPC::Stop()
{

}

bool AShipEquipPC::AsteroidControl_Validate()
{
	return true;
}

void AShipEquipPC::AsteroidControl_Implementation()
{
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->IsA<AAsteroidSpawnManager>())
		{
			AAsteroidSpawnManager *pManager = Cast<AAsteroidSpawnManager>(*ActorItr);
			pManager->IsDisabled = !pManager->IsDisabled;
		}
	}
}

void AShipEquipPC::SetVisibility_Implementation(ATowardsTheStarsCharacter* pCharacter, bool Visibility)
{
	pCharacter->CurrentEquipment->EquipmentMesh->SetVisibility(Visibility);
	pCharacter->CurrentEquipment->EquipmentMeshThirdPerson->SetVisibility(Visibility);
	pCharacter->Mesh->SetVisibility(Visibility);
	pCharacter->FPMesh->SetVisibility(Visibility);
}

#pragma region ChatSystem
bool AShipEquipPC::ChatInput_Turret_Validate(const FString& text)
{
	return true;
}

void AShipEquipPC::ChatInput_Turret_Implementation(const FString& text)
{
	SetChatInput_Turret(text);
}

void AShipEquipPC::SetChatInput_Turret_Implementation(const FString& text)
{
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (ActorItr->IsA<ATowardsTheStarsCharacter>())
		{
			ATowardsTheStarsCharacter *pCharacter = Cast<ATowardsTheStarsCharacter>(*ActorItr);
			if (pCharacter->IsLocallyControlled())
			{
				pCharacter->MessageToSend = text;
				pCharacter->needsToUpdateChat = true;
			}
		}
		else if (ActorItr->IsA<AShipEquipPC>())
		{
			AShipEquipPC *pEquip = Cast<AShipEquipPC>(*ActorItr);
			if (pEquip->IsLocallyControlled())
			{
				pEquip->MessageToSend = text;
				pEquip->needsToUpdateChat = true;
			}
		}
	}
}

void AShipEquipPC::UpdateTextChatContent_Turret()
{
	ChatHistory.Add(MessageToSend);
	UpdateTextChat_Turret(MessageToSend);
	needsToUpdateChat = false;
	//DebugPrint("Secondary Function Released");
}

void AShipEquipPC::UpdateTextChat_Turret_Implementation(const FString& text)
{

}

void AShipEquipPC::SetAlert_Turret_Implementation(const FString& text)
{

}
#pragma endregion ChatSystem

#pragma region AlertSystem

bool AShipEquipPC::OnReceiveAnnouncement(FAnnouncement a_Announcement)
{
	SetChatInput_Turret(a_Announcement.m_strMessage);
	if (a_Announcement.m_Level == eAnnouncementLevel_Severe)
	{
		SetAlert_Turret(a_Announcement.m_strMessage);
	}
	return true;
}

bool AShipEquipPC::RegisterAnnouncement_Validate(eAnnouncementLocation e)
{
	return true;
}

void AShipEquipPC::RegisterAnnouncement_Implementation(eAnnouncementLocation e)
{
	Cast<ATowardsTheStarsGameMode>(GetWorld()->GetAuthGameMode())->AnnouncementManager->RegisterListener(this, e);
}
#pragma endregion AlertSystem