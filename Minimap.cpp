// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "TowardsTheStarsCharacter.h"
#include "PlayerHUD.h"
#include "ShipSystem.h"
#include "UnrealNetwork.h"
#include "Minimap.h"

/*Global Pointer*/
static AMinimap* MinimapInstance = NULL;

AMinimap::AMinimap(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	CutoffZValue = 500.0f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	DisplayPos1.X = -100.0f;
	DisplayPos2.X = -100.0f;
	WorldX = 8400;
	WorldY = 5225;
	WorldStartX = 5700.0f;
	WorldStartY = 2428.0f;
	XOffset = 0.0f;
	YOffset = 0.0f;
	XWidth = 382.0f;
	YWidth = 227.0f;
}

void AMinimap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMinimap, PlayerPositions);
	DOREPLIFETIME(AMinimap, CharacterNumber);
	DOREPLIFETIME(AMinimap, Players);
}

void AMinimap::OnInstantiation(AMinimap* m){
	MinimapInstance = m;
	GetWorldTimerManager().SetTimer(this, &AMinimap::GetPlayerInformation, 1.0f, false);
	//SetHUD();
	//GetPlayerInformation();
}

void AMinimap::SetCoordinates(FVector2D one, FVector2D two){
	DisplayPos1 = one;
	DisplayPos2 = two;
}

void AMinimap::GetPlayerInformation(){
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATowardsTheStarsCharacter::StaticClass(), Players);
	int32 i = 0;
	for (auto Itr(Players.CreateIterator()); Itr; Itr++)
	{
		ATowardsTheStarsCharacter* c = Cast<ATowardsTheStarsCharacter>(*Itr);
		Characters.Add(c);
		i++;
	}
	CharacterNumber = i;
	//DebugPrint(FString::Printf(TEXT("[Minimap] Character Number: %f"), CharacterNumber));
	
}

AMinimap* AMinimap::GetMinimap(){
	if (MinimapInstance){
		DebugPrint(FString::Printf(TEXT("[Minimap] Minimap Instance is not null")));
	}
	return MinimapInstance;
}

void AMinimap::BeginPlay(){
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATowardsTheStarsCharacter::StaticClass(), Players);
	int32 i = 0;
	for (auto Itr(Players.CreateIterator()); Itr; Itr++)
	{
		ATowardsTheStarsCharacter* c = Cast<ATowardsTheStarsCharacter>(*Itr);
		if (!c->isAI)
		{
			Characters.Add(c);
			i++;
		}
	}
	CharacterNumber = i;
}

void AMinimap::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	UpdatePlayerLocation();
}

void AMinimap::UpdatePlayerLocation(){
	//DebugPrint(FString::Printf(TEXT("[Minimap] Update Location")));
	ENetMode t = GetWorld()->GetNetMode();
	int i = 0;
	/*if (MyHUD){
		MyHUD->SetPlayerNumber(CharacterNumber);
	}*/
	if (CharacterNumber == 0){
		GetPlayerInformation();
	}
	SetPlayerNumber(CharacterNumber);

	if (PlayerPositions.Num() != Players.Num()){
		PlayerPositions.Init(FVector(-10.0f, -10.0f, 0.0f), Players.Num());
	}
	i = 0;
	for (auto Itr(Players.CreateIterator()); Itr; Itr++)
	{
		ATowardsTheStarsCharacter* c = Cast<ATowardsTheStarsCharacter>(*Itr);
		if(c && (!c->isAI))
		{
			FVector loc = c->GetActorLocation();
			float px;
			float py;
			//Map ox, oy to correct new x and y, and draw
			px = DisplayPos2.X + XOffset + (loc.X + WorldStartX)*XWidth / WorldX;
			py = DisplayPos2.Y + YOffset + (loc.Y + WorldStartY)*YWidth / WorldY;

			if (loc.Z > CutoffZValue){
				//Draw on L1 Texture
				//MyHUD->DrawPlayer(loc.X, loc.Y, true, i);
				px = DisplayPos2.X + XOffset + (loc.X + WorldStartX)*XWidth / WorldX;
				py = DisplayPos2.Y + YOffset + (loc.Y + WorldStartY)*YWidth / WorldY;
				//DebugPrint(FString::Printf(TEXT("X: %f, Y: %f"), px, py));
				PlayerPositions[i] = FVector(px, py, 1.0f);
			}
			else{
				//Draw on L2 Texture
				//MyHUD->DrawPlayer(loc.X, loc.Y, false, i);
				if (DisplayPos1.X > 0){
					px = DisplayPos1.X + XOffset + (loc.X + WorldStartX)*XWidth / WorldX;
					py = DisplayPos1.Y + YOffset + (loc.Y + WorldStartY)*YWidth / WorldY;
					//DebugPrint(FString::Printf(TEXT("X: %f, Y: %f"), px, py));
					PlayerPositions[i] = FVector(px, py, 0.0f);
				}
			}
			i++;
		}
	}
}

void AMinimap::SetPlayerNumber_Implementation(int32 Num){

}


