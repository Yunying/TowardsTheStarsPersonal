// Fill out your copyright notice in the Description page of Project Settings.

#include "TowardsTheStars.h"
#include "Minimap.h"
#include "MinimapHUD.h"


AMinimapHUD::AMinimapHUD(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> MMObj(TEXT("Texture2D'/Game/Textures/Minimap/L1.L1'"));
	MinimapL1 = MMObj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> MM2Obj(TEXT("Texture2D'/Game/Textures/Minimap/L2.L2'"));
	MinimapL2 = MM2Obj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> MM3Obj(TEXT("TextureRenderTarget2D'/Game/Textures/Minimap/MushroomIcon.MushroomIcon'"));
	PlayerIcon = MM3Obj.Object;

	DisplayPos1.X = -100.0f;
	DisplayPos2.X = -100.0f;
	WorldX = 8110;
	WorldY = 4295;
	WorldStartX = 1458.0f;
	WorldStartY = 1948.0f;
	XOffset = 10.0f;
	YOffset = 72.0f;
	XWidth = 235.0f;
	YWidth = 120.0f;
}

void AMinimapHUD::BeginPlay(){
	AMinimap* map = AMinimap::GetMinimap();
	//map->SetHUD(this);
}

void AMinimapHUD::SetPlayerNumber(int32 num){
	PlayerNumber = num;
}

void AMinimapHUD::DrawHUD(){
	Super::DrawHUD();
	DisplayPos1 = FVector2D(Canvas->ClipX * 0.2f, Canvas->ClipY * 0.6f);
	DisplayPos2 = FVector2D(Canvas->ClipX * 0.6f, Canvas->ClipY * 0.6f);
	AMinimap* map = AMinimap::GetMinimap();
	map->SetCoordinates(DisplayPos1, DisplayPos2);
	DrawMinimapTexture(DisplayPos1, DisplayPos2);
	DrawPlayer();
	DrawPlayers();
}

void AMinimapHUD::DrawMinimapTexture(FVector2D DisplayPos1, FVector2D DisplayPos2){
	//FCanvasTileItem TextureItem(FVector2D(0.0f, 0.0f), MinimapL1->Resource, FVector2D(0.0f, 0.3f), FVector2D(1.0f, 1.0f), FLinearColor::White);
	//TextureItem.BlendMode = SE_BLEND_Opaque;
	//Canvas->DrawItem(TextureItem, DisplayPos1);
	DrawTexture(MinimapL1, DisplayPos1.X, DisplayPos1.Y, MinimapL1->GetSizeX(), MinimapL1->GetSizeY(), 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
	
	DrawTexture(MinimapL2, DisplayPos2.X, DisplayPos2.Y, MinimapL2->GetSizeX(), MinimapL2->GetSizeY(), 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
}

void AMinimapHUD::DrawPlayer(){
	if (PlayerPositions.Num() == 0){
		PlayerPositions.Init(FVector(0, 0, 0.0f), PlayerNumber);
	}
	else{
		AMinimap* map = AMinimap::GetMinimap();
		for (int i = 0; i < PlayerNumber; i++){
			PlayerPositions[i] = map->PlayerPositions[i];
			//DebugPrint(FString::Printf(TEXT("X: %f, Y: %f"), PlayerPositions[i].X, PlayerPositions[i].Y));
		}
	}

}

void AMinimapHUD::DrawPlayers(){
	for (auto Itr(PlayerPositions.CreateIterator()); Itr; Itr++)
	{
		FVector pos = *Itr;
		if (pos.Z == 1){
			DrawTexture(PlayerIcon, pos.X, pos.Y, 8, 8, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
			//PlayerPositions.Remove(pos);
		}
		else{
			DrawTexture(PlayerIcon, pos.X, pos.Y, 8, 8, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
			//PlayerPositions.Remove(pos);
		}

	}
}

