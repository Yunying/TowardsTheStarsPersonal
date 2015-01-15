

#include "TowardsTheStars.h"
#include "PlayerHUD.h"
#include "Turret.h"
#include "RapidTurret.h"
#include "Item.h"
#include "Scanner.h"
#include "RepairTool.h"

APlayerHUD::APlayerHUD(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UFont>HUDFontOb(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = HUDFontOb.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> TurretCrosshairObj(TEXT("Texture2D'/Game/Textures/PlayerHud/CrossHair.CrossHair'"));
	TurretCrossHairTex = TurretCrosshairObj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> TurretAmmoObj(TEXT("Texture2D'/Game/Textures/PlayerHud/Ammo.Ammo'"));
	TurretAmmoTex = TurretAmmoObj.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairObj(TEXT("Texture2D'/Game/Textures/PlayerHud/Crosshair_red.Crosshair_red'"));
	CrosshairTex = CrosshairObj.Object;

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> MinimapObj(TEXT("TextureRenderTarget2D'/Game/Textures/PlayerHud/Minimap.Minimap'"));
	MinimapTex = MinimapObj.Object;

	inTurret = false;
	inScanner = false;
	TurretAmmo = 0;
	TurretCharge = -1;
	Scanning = false;
}

void APlayerHUD::DrawHUD()
{
	FVector2D ScreenDimensions = FVector2D(Canvas->SizeX, Canvas->SizeY);

	Super::DrawHUD();

	if (inTurret)
	{
		//DrawText("Push E to exit", FColor::White, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.1f, HUDFont, 1.0f);

		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
		const FVector2D CrosshairDrawPosition((Center.X - (TurretCrossHairTex->GetSurfaceWidth() * 0.5)),
			(Center.Y - (TurretCrossHairTex->GetSurfaceHeight() * 0.5f)));
		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, TurretCrossHairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);

		if (RapidTurret)
		{
			FString name = "Current Charge: " + FString::Printf(TEXT("%10.1f"), FMath::Abs(TurretCharge)) + "/3.0";
			DrawText(name, FColor::White, 250, 250, HUDFont);
		}
		else
		{
			FVector2D AmmoPos(Canvas->ClipX * 0.15f, Canvas->ClipY - Canvas->ClipY * 0.15f);
			FVector2D AmmoDrawPosition((AmmoPos.X - (TurretAmmoTex->GetSurfaceWidth() * 0.5)),
				(AmmoPos.Y - (TurretAmmoTex->GetSurfaceHeight() * 0.5f)));
			for (int i = 0; i < TurretAmmo; i++)
			{
				FCanvasTileItem AmmoItem(AmmoDrawPosition, TurretAmmoTex->Resource, FLinearColor::White);
				AmmoItem.Size = AmmoItem.Size*.5f;
				AmmoItem.BlendMode = SE_BLEND_Translucent;
				Canvas->DrawItem(AmmoItem);
				AmmoDrawPosition.Y -= 50;
			}
		}
	}

	else if (inScanner){
		AScanner* sc = Cast<AScanner>(GetOwningPlayerController()->GetPawn());
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
		const FVector2D CrosshairDrawPosition((Center.X - (TurretCrossHairTex->GetSurfaceWidth() * 0.5)),
			(Center.Y - (TurretCrossHairTex->GetSurfaceHeight() * 0.5f)));
		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, TurretCrossHairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
		ShowDetectedAsteroidInformation(sc->SelectedAsteroid);

		//To determine which text information to show
		
		if (sc){
			if (Scanning){
				//DebugPrint(FString::Printf(TEXT("HUD: Scanning")));
				ShowScanningTimeCountDown();
			}
			else{
				//DebugPrint(FString::Printf(TEXT("HUD Normal")));
				ScanTime = sc->ScanTime;
				ShowDetectedAsteroidInformation(sc->SelectedAsteroid);
			}
		}
		
	}


	else //Normal Player Character HUD
	{
		ATowardsTheStarsCharacter* pCharacter = Cast<ATowardsTheStarsCharacter>(GetOwningPawn());
		if (pCharacter)
		{
			//HP
			FString HPString = FString::Printf(TEXT("HP %3.0f"), pCharacter->HP);
			DrawText(HPString, FColor::White, 50, 50, HUDFont, 2.0f);
			//
			ShowDetectedObjectInformationV2(pCharacter->SelectedActor);
			//
			ShowAmmoV2(pCharacter);
			//
			ShowCurrentItemV2(pCharacter);

			//Draw Minimap
			//DrawMinimap();

			int k = 0;
			for (auto it = pCharacter->ChatHistory.CreateIterator(); it; it++, ++k)
			{
				DrawText((*it), FColor::White, 50, 100 + 20 * k, HUDFont);
			}
			
		}

		//Target Cursor
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
		const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
			(Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)));
		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);

		
	}

}
void APlayerHUD::ShowDetectedObjectInformationV2(AActor* SelectedActor)
{
	ATowardsTheStarsCharacter* pCharacter = Cast<ATowardsTheStarsCharacter>(GetOwningPawn());
	if (pCharacter)
	{
		const FVector2D TextLocation(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.1f);
		const int DescriptionYOffSet = 20;
		const float EquipmentFontSize = 1.0f;
		DrawText(pCharacter->DetectedObjectInformation1, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
		DrawText(pCharacter->DetectedObjectInformation2, FColor::White, TextLocation.X, TextLocation.Y + DescriptionYOffSet, HUDFont);
	}
}

void APlayerHUD::ShowDetectedObjectInformation(AActor* SelectedActor)
{

	FString display;

	if (SelectedActor != NULL)
	{
		//TODO: This should be factored out?
		const FVector2D TextLocation(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.1f);
		const int DescriptionYOffSet = 20;
		const float EquipmentFontSize = 1.0f;
		if (SelectedActor->IsA<APlayerEquipment>())
		{
			APlayerEquipment *pEquipment = Cast<APlayerEquipment>(SelectedActor);
			FString display = pEquipment->ObjectName + " " + FString::Printf(TEXT("%3.0f"), FMath::Abs(pEquipment->getCurrentHealth())) + "/" + FString::Printf(TEXT("%3.0f"), FMath::Abs(pEquipment->getMaximumHealth()));
			DrawText(display, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
			DrawText(pEquipment->Discreption, FColor::White, TextLocation.X, TextLocation.Y + DescriptionYOffSet, HUDFont);
		}
		else if (SelectedActor->IsA<AItem>())
		{
			AItem *pItem = Cast<AItem>(SelectedActor);
			FString display = pItem->ObjectName + " " + FString::Printf(TEXT("%3.0f"), FMath::Abs(pItem->getCurrentHealth())) + "/" + FString::Printf(TEXT("%3.0f"), FMath::Abs(pItem->getMaximumHealth()));
			DrawText(display, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
			DrawText(pItem->Discreption, FColor::White, TextLocation.X, TextLocation.Y + DescriptionYOffSet, HUDFont);
		}
		else if (SelectedActor->IsA<AShipEquipNPC>())
		{
			AShipEquipNPC *pObj = Cast<AShipEquipNPC>(SelectedActor);
			FString name = pObj->ObjectName + " " + FString::Printf(TEXT("%3.0f"), FMath::Abs(pObj->CurrentHealth)) + "/" + FString::Printf(TEXT("%3.0f"), FMath::Abs(pObj->MaxHealth));
			DrawText(name, FColor::White, TextLocation.X, TextLocation.Y, HUDFont, EquipmentFontSize);
			FString description = pObj->ObjDescription;
			DrawText(description, FColor::White, TextLocation.X, TextLocation.Y + DescriptionYOffSet, HUDFont, EquipmentFontSize);
		}
		else if (SelectedActor->IsA<AShipEquipPC>())
		{
			AShipEquipPC *pObj = Cast<AShipEquipPC>(SelectedActor);
			FString name = pObj->ObjectName + " " + FString::Printf(TEXT("%3.0f"), FMath::Abs(pObj->CurrentHealth));;
			DrawText(name, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
		}
	}

	else
	{
		DrawText("", FColor::White, 250, 250, HUDFont);
	}
}

void APlayerHUD::ShowAmmoV2(ATowardsTheStarsCharacter* pCharacter)
{
	
	const FVector2D WeaponNameDrawPos(Canvas->ClipX * 0.85f, Canvas->ClipY * 0.85f);
	const FVector2D AmmoDrawPos(Canvas->ClipX * 0.85f, Canvas->ClipY * 0.9f);
	DrawText(pCharacter->CurrentEquipmentInformation, FColor::White, WeaponNameDrawPos.X, WeaponNameDrawPos.Y, HUDFont);
	DrawText(pCharacter->CurrentAmmoInformation + "/" + pCharacter->TotalAmmoInformation, FColor::White, AmmoDrawPos.X, AmmoDrawPos.Y, HUDFont);
	if (pCharacter->IsLowAmmo())
	{
		DrawText("Out of Ammo", FColor::Red, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f, HUDFont);
	}
	
}

void APlayerHUD::ShowAmmo(ATowardsTheStarsCharacter* pCharacter)
{
	const FVector2D WeaponNameDrawPos(Canvas->ClipX * 0.85f, Canvas->ClipY * 0.85f);
	if (pCharacter->CurrentEquipment != NULL && !pCharacter->CurrentEquipment->IsA<ARepairTool>())
	{
		//const FVector2D WeaponNameDrawPos(Canvas->ClipX * 0.9f, Canvas->ClipY * 0.85f);
		const FVector2D AmmoDrawPos(Canvas->ClipX * 0.85f, Canvas->ClipY * 0.9f);
		DrawText(pCharacter->CurrentEquipment->ObjectName, FColor::White, WeaponNameDrawPos.X, WeaponNameDrawPos.Y, HUDFont);
		DrawText(FString::Printf(TEXT("%d/%d"), pCharacter->CurrentEquipment->iCurrentAmmo, pCharacter->CurrentEquipment->iClipAmmo), FColor::White, AmmoDrawPos.X, AmmoDrawPos.Y, HUDFont);
		if (pCharacter->IsLowAmmo())
		{
			DrawText("Out of Ammo", FColor::Red, Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f, HUDFont);
		}
	}
	else if (pCharacter->CurrentEquipment != NULL && pCharacter->CurrentEquipment->IsA<ARepairTool>())
	{
		const FVector2D AmmoDrawPos(Canvas->ClipX * 0.85f, Canvas->ClipY * 0.9f);
		DrawText(pCharacter->CurrentEquipment->ObjectName, FColor::White, WeaponNameDrawPos.X, WeaponNameDrawPos.Y, HUDFont);
	}
	else if (pCharacter->CurrentEquipment == NULL)
	{
		DrawText(FString::Printf(TEXT("No Equipment")), FColor::White, WeaponNameDrawPos.X, WeaponNameDrawPos.Y, HUDFont);
	}
}

void APlayerHUD::ChangeHUD(bool Exit)
{
	if (Exit)
	{
		inTurret = false;
		inScanner = false;
	}
	else if (GetOwningPlayerController()->GetPawn()->IsA(ATurret::StaticClass()))
	{
		inTurret = true;
		ATurret* turret = Cast<ATurret>(GetOwningPlayerController()->GetPawn());
		TurretAmmo = turret->currentAmmo;
	}
	else if (GetOwningPlayerController()->GetPawn()->IsA(ARapidTurret::StaticClass()))
	{
		inTurret = true;
		ARapidTurret* turret = Cast<ARapidTurret>(GetOwningPlayerController()->GetPawn());
		TurretCharge = turret->currentChargeLevel;
		RapidTurret = true;
	}
	else if (GetOwningPlayerController()->GetPawn()->IsA(AScanner::StaticClass()))
	{
		inScanner = true;
		AScanner* sc = Cast<AScanner>(GetOwningPlayerController()->GetPawn());
		sc->OnEnter();
		DebugPrint(FString::Printf(TEXT("Enter Scanner HUD")));
	}
}

void APlayerHUD::ShowCurrentItemV2(ATowardsTheStarsCharacter* pCharacter)
{
	const FVector2D DisplayPos(Canvas->ClipX * 0.1f, Canvas->ClipY * 0.85f);
	const FVector2D ItemDrawPos(Canvas->ClipX * 0.1f, Canvas->ClipY * 0.9f);
	DrawText("Current Item", FColor::White, DisplayPos.X, DisplayPos.Y, HUDFont);
	DrawText(pCharacter->CurrentItemInformation, FColor::White, ItemDrawPos.X, ItemDrawPos.Y, HUDFont);
}


void APlayerHUD::ShowCurrentItem(ATowardsTheStarsCharacter* pCharacter)
{
	const FVector2D DisplayPos(Canvas->ClipX * 0.1f, Canvas->ClipY * 0.85f);
	const FVector2D ItemDrawPos(Canvas->ClipX * 0.1f, Canvas->ClipY * 0.9f);
	DrawText("Current Item", FColor::White, DisplayPos.X, DisplayPos.Y, HUDFont);
	if (pCharacter->GetCurrentItem() != NULL)
	{
		DrawText(pCharacter->GetCurrentItem()->ObjectName, FColor::White, ItemDrawPos.X, ItemDrawPos.Y, HUDFont);
	}
}

void APlayerHUD::DrawMinimap(){
	/*if (Map){
		if (Map->MyHUD){
			Map->MyHUD->DrawHUD();
		}
		else{

		}
		DebugPrint(FString::Printf(TEXT("Draw Mappppppp")));
	}
	else{
		Map = AMinimap::GetMinimap();
		DebugPrint(FString::Printf(TEXT("No Mapppppp")));
	}*/
}

void APlayerHUD::ShowDetectedAsteroidInformation(AActor* SelectedActor){
	if (SelectedActor != NULL)
	{
		//TODO: This should be factored out?
		const FVector2D TextLocation(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.1f);
		const int DescriptionYOffSet = 20;
		const float EquipmentFontSize = 1.0f;
		if (SelectedActor->IsA<AAsteroidBase>())
		{
			AAsteroidBase *pAsteroid = Cast<AAsteroidBase>(SelectedActor);
			FString display = FString::Printf(TEXT("Asteroid Health: %f"), pAsteroid->mHealth);

			DrawText(display, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
		}
	}

	else
	{
		DrawText("", FColor::White, 250, 250, HUDFont);
	}
}

void APlayerHUD::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);
	if (Scanning){
		ScanTime -= DeltaSeconds;
	}
	if (inScanner){
		AScanner* sc = Cast<AScanner>(GetOwningPlayerController()->GetPawn());
		if (sc->ScannerState == EScannerState::SCANNING){
			Scanning = true;
			//DebugPrint(FString::Printf(TEXT("HUD: Scanning")));
		}
		else{
			Scanning = false;
			//DebugPrint(FString::Printf(TEXT("HUD: Normal")));
		}
	}
}

void APlayerHUD::ShowScanningTimeCountDown(){
	//DebugPrint(FString::Printf(TEXT("Show Count Down")));
	const FVector2D TextLocation(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.1f);
	FString display = "ScanningTime: " + FString::Printf(TEXT("%3.0f"), ScanTime);
	DrawText(display, FColor::White, TextLocation.X, TextLocation.Y, HUDFont);
}

void APlayerHUD::SetScanning(bool s){
	Scanning = s;
	//DebugPrint(FString::Printf(TEXT("Set Scanning")));
}