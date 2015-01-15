

#include "TowardsTheStars.h"
#include "Elevator.h"


AElevator::AElevator(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP),
	m_bStartedCue(false)
{
	Elevator = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ElevatorMesh"));
	Elevator->AttachTo(RootComponent);

	Velocity = 200.0f;
	FloorHeight = 200.0f;
	ElevatorState = EElevatorEnum::DOWNSTAIRS;
	ObjDescription = "Going Up?";
	EnergyUsage = 0.0f;
}

void AElevator::BeginPlay()
{
	CurrentLocation = GetActorLocation();
	NewLocation = FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z + FloorHeight);

	DownstairsLocation = CurrentLocation;
	UpstairsLocation = NewLocation;

	if (!MyTopConsole)
	{
		DebugPrint(FString::Printf(TEXT("[Elevator] Missing Top Console for the Elevator")));
	}
	if (!MyBottomConsole)
	{
		DebugPrint(FString::Printf(TEXT("[Elevator] Missing Bottom Console for the Elevator")));
	}
}

void AElevator::SetMoveLocation(){
	if (ElevatorState == EElevatorEnum::DOWNSTAIRS){
		CurrentLocation = DownstairsLocation;
		NewLocation = UpstairsLocation;
	}
	else if (ElevatorState == EElevatorEnum::UPSTAIRS){
		CurrentLocation = UpstairsLocation;
		NewLocation = DownstairsLocation;
	}
}

void AElevator::UseEnergy()
{
	if (Sys){
		DebugPrint(FString::Printf(TEXT("Ship System Available. Used Energy")));
		Sys->UseEnergy(EnergyUsage);
	}
	else{
		DebugPrint(FString::Printf(TEXT("Ship System Not Available. Set Ship System To Use Energy")));
		Sys = AShipSystem::GetShipSystem();
		Sys->UseEnergy(EnergyUsage);
	}
}

//Notify Top Console of the new elevator state
void AElevator::NotifyTopConsole()
{
	if (MyTopConsole){
		MyTopConsole->msgElevatorArrived();
	}
	else{
		DebugPrint(FString::Printf(TEXT("[Elevator] Missing Top Console for the Elevator")));
	}
}

//Notify Bottom Console of the new elevator state
void AElevator::NotifyBottomConsole()
{
	if (MyBottomConsole){
		MyBottomConsole->msgElevatorArrived();
	}
	else{
		DebugPrint(FString::Printf(TEXT("[Elevator] Missing Bottom Console for the Elevator")));
	}
}

void AElevator::Tick(float DeltaSeconds){
	Super::Tick(DeltaSeconds);

	if (ElevatorState == EElevatorEnum::MOVINGUP){
		// Check if Elevator has arrived
		if (GetActorLocation() == NewLocation){
			EndMove();
			return;
		}

		else{
			if (!m_bStartedCue)
			{
				PlayCue();
			}
			if (GetActorLocation().Z + Velocity*DeltaSeconds > NewLocation.Z){
				SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewLocation.Z), false);
			}
			else{
				SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, GetActorLocation().Z + Velocity*DeltaSeconds), false);
			}
		}
	}

	else if (ElevatorState == EElevatorEnum::MOVINGDOWN){
		if (GetActorLocation() == NewLocation){
			EndMove();
			return;
		}

		else{
			if (!m_bStartedCue)
			{
				PlayCue();
			}
			if (GetActorLocation().Z - Velocity*DeltaSeconds < NewLocation.Z){
				SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewLocation.Z), false);
			}
			else{
				SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, GetActorLocation().Z - Velocity*DeltaSeconds), false);
			}
		}
	}
}

void AElevator::EndMove(){

	UseEnergy();

	switch (ElevatorState){
		case(EElevatorEnum::MOVINGUP):
			ElevatorState = EElevatorEnum::UPSTAIRS;
			NotifyTopConsole();
			NewLocation = DownstairsLocation;
			CurrentLocation = UpstairsLocation;
			stateChanged();
			break;
		
		case(EElevatorEnum::MOVINGDOWN):
			ElevatorState = EElevatorEnum::DOWNSTAIRS;
			NotifyBottomConsole();
			NewLocation = UpstairsLocation;
			CurrentLocation = DownstairsLocation;
			stateChanged();
			break;
		
	}
	stateChanged();

	if (MovingAudioComponent)
		MovingAudioComponent->Stop();
	if (ArrivedCue)
		UGameplayStatics::PlaySoundAttached(ArrivedCue, RootComponent);
	m_bStartedCue = false;
}

void AElevator::PlayCue()
{
	m_bStartedCue = true;
	if (MovingCue)
		MovingAudioComponent = UGameplayStatics::PlaySoundAttached(MovingCue, RootComponent);
}

void AElevator::msgConsoleCall(FString s){
	if (s.Equals(FString("Up"))){
		// Add an event to the event list; true means a player is waiting upstairs
		events.push_back(true);
		stateChanged();
	}
	else{
		// Add an event to the event list; false means a player is waiting downstairs
		events.push_back(false);
		stateChanged();
	}
}

void AElevator::stateChanged(){
	if (!events.empty()){
		switch (ElevatorState){
			// If the elevator is staying upstairs, then delete the events in the list that calls the elevator to go upstairs
			case (EElevatorEnum::UPSTAIRS):
				for (int i = 0; i < events.size(); i++){
					if (events.at(i) == true){
						events.erase(events.begin()+i);
						NotifyTopConsole();
					}
				}
				for (int i = 0; i < events.size(); i++){
					if (events.at(i) == false){
						events.erase(events.begin() + i);
						ElevatorState = EElevatorEnum::MOVINGDOWN;
					}
				}
				break;

			case (EElevatorEnum::DOWNSTAIRS) :
				for (int i = 0; i < events.size(); i++){
					if (events.at(i) == false){
						events.erase(events.begin() + i);
						NotifyBottomConsole();
					}
				}
				for (int i = 0; i < events.size(); i++){
					if (events.at(i) == true){
						events.erase(events.begin() + i);
						ElevatorState = EElevatorEnum::MOVINGUP;
					}
				}
				break;
		}
	}
		
}

