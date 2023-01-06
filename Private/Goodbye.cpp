// Fill out your copyright notice in the Description page of Project Settings.


#include "Goodbye.h"

UGoodbye::UGoodbye()
{
	Guide = Cast<UVHGuide>(GetOuter());
	if (IsValid(Guide)) Guide->PrintDebug("State Farewell created");
}

void UGoodbye::Tick(float DeltaTime)
{
}

void UGoodbye::OnStateChange()
{
	Followers = Guide->Followers;
	// play goodbye dialogue
	Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.Goodbye, 0.1, true);
}

void UGoodbye::GazeModel(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;
	int i;
	float AgentToUserAngle;

	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeState)
		{
		case EGazeState::ToUser:
			Guide->GazingComp->GazeToUser();
			GazePhaseDuration = FMath::FRandRange(2.0, 6.0);
			GazeState = EGazeState::ToParticipant;
			break;
		case EGazeState::ToPoI:
			//Should not be called
			break;
		case EGazeState::ToParticipant:
			i = FMath::RandRange(0, 2);
			AgentToUserAngle = FVector::DotProduct(Guide->VH->GetActorForwardVector(), Followers[i]->GetActorLocation() - Guide->VH->GetActorLocation());
			AgentToUserAngle = FMath::Acos(AgentToUserAngle);
			AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);
			if (AgentToUserAngle <= PeripheralVisionAngle) {
				Guide->GazingComp->GazeToActor(Followers[i], FVector(0, 0, 120.0f));
				GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
				GazeState = EGazeState::ToUser;
			}
			else {
				GazeState = EGazeState::ToUser;
			}
			break;
		case EGazeState::Ahead:
			// should not be called
			GazeState = EGazeState::ToUser;
			break;
		case EGazeState::AtRandom:
			//should not be called
			GazeState = EGazeState::Ahead;
			break;
		default:
			//VH_ERROR("[VHGuide::GazeModel] Unknown Gaze state!\n");
			break;
		}
	}
}