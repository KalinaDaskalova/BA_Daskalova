// Fill out your copyright notice in the Description page of Project Settings.
#include "AtPoI.h"
#include "ToPoI.h"
#include "Engine.h"

// Sets default values
UAtPoI::UAtPoI()
{
	Guide = Cast<UVHGuide>(GetOuter());
	if (IsValid(Guide)) Guide->PrintDebug("State UAtPoi created");

}


// Called every frame
void UAtPoI::Tick(float DeltaTime)
{
	WaitForUserTimeElapsed += DeltaTime;
	AtPoITimeElapsed += DeltaTime;
	switch (PoIState)
	{
	case (ApproachingPoI):
		//Move Guide and Participants to Points
		if (bDoOnceTwo) {
			GuideMoveToPosition();
			GetNearestPosition(GuidePositionLocation);
			FormationPositions = Guide->ToPoI->PositionsPointer;
			if (!FormationPositions.Num() == 0)
			{
				for (int i = 0; i < FormationPositions.Num(); i++)
				{
					if (FormationPositions[i] == Malika->AssignedSlot)
					{
						MalikaPosition = GetFurthestPosition(Cast<AVirtualHuman>(Malika));
						AIController = Cast<AVirtualHumanAIController>((Malika->GetController()));
						AIController->MoveToLocation(MalikaPosition, 10.0f, true);
					}
					if (FormationPositions[i] == Omar->AssignedSlot)
					{
						OmarPosition = GetFurthestPosition(Cast<AVirtualHuman>(Omar));
						AIController = Cast<AVirtualHumanAIController>((Omar->GetController()));
						AIController->MoveToLocation(OmarPosition, 10.0f, true);
					}
					if (FormationPositions[i] == Vivian->AssignedSlot)
					{
						VivianPosition = GetFurthestPosition(Cast<AVirtualHuman>(Vivian));
						AIController = Cast<AVirtualHumanAIController>((Vivian->GetController()));
						AIController->MoveToLocation(VivianPosition, 10.0f, true);
					}
				}
			}
			else {
				for (int i = 0; i < Guide->Followers.Num(); i++)
				{
					FVector PointLocation = GetNearestPosition(Cast<AVirtualHuman>(Followers[i]));
					AIController = Cast<AVirtualHumanAIController>((Followers[i]->GetController()));
					AIController->MoveToLocation(PointLocation, 10.0f, true);
					CheckUserLocation();
				}
			}
			bDoOnceTwo = false;
		}
		PoIState = AtPoi;
		break;
	case (AtPoi):
		//If guide is not speaking and user is still not at PoI
		if (!CheckUserLocation() && AtPoITimeElapsed >= 15.0f && !bUserCalled && !bGuideIsSpeaking)
		{
			OnWaitForUser();
		}
		//If guide is already speaking and user decides to get away from the group
		if (!CheckUserLocation() && AtPoITimeElapsed >= 15.0f && !bUserCalled && bGuideIsSpeaking)
		{
			Malika->GazingComp->GazeToUser();
			Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.HeyComeToTheGroup, 0.1f, true);
			bUserCalled = true;
		}
		else if (CheckUserLocation())
		{
			WaitForUserTimeElapsed = 0.0f;
			if (bDoOnce) {
				Guide->PrintDebug("UserAtPoI");
				GiveInformation();
				bDoOnce = false;
			}
		}
		if ((Guide->SpeechComp->bIsSpeaking))
		{
			bGuideIsSpeaking = true;
		}
		if (bGuideIsSpeaking)
		{
			if (!(Guide->SpeechComp->bIsSpeaking))
			{
				PoIState = Finished;
				bGuideIsSpeaking = false;
			}
		}
		break;
	case (WaitForUser):
		//Turn to user and tell them to get to the group
		if (!CheckUserLocation() && WaitForUserTimeElapsed >= 30)
		{
			if (bDoOnce) {
				GiveInformation();
				Malika->GazingComp->GazeToUser();
				Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.HeyComeToTheGroup, 0.1f, true);
				bDoOnce = false;
				PoIState = AtPoi;
			}
		}
		else if (CheckUserLocation())
		{
			WaitForUserTimeElapsed = 0.0f;
			Guide->PrintDebug("UserAtPoI");
			if (bDoOnce) {
				GiveInformation();
				PoIState = AtPoi;
				bDoOnce = false;
			}
		}
		break;
	case (Finished):
		//Call function Okaay lets go! and change state to ToPoI
		Guide->ToPoI->bDoOnceThree = true;
		Guide->NextPoI();
		//Guide->ChangeState(Guide->ToPoI);
		break;
	}
}
	


void UAtPoI::OnStateChange()
{
	//Guide->LogToFile(FString::Printf(TEXT("AtPoi: %s"), *Guide->GetPoi()->GetName()));
	PointsFree.Empty();
	Positions.Empty();
	for (int i = 0; i < 5; i++)
	{
		PointsFree.Add(true);
	}
	GazePhaseDuration = 0;
	WaitForUserTimeElapsed = 0;
	//Participant = Cast<AVHParticipant>(UGameplayStatics::GetActorOfClass(GetWorld(), AVHParticipant::StaticClass()));
	Followers = Guide->Followers;
	Guide->PrintDebug("State: AtPoi\n");
	PoIState = ApproachingPoI;
	Positions = Guide->CurrentPoI->InitStandingPositions();
	GazeState = EGazeState::ToPoI;	
	bGuideIsSpeaking = false;
	bDoOnce = true;
	bDoOnceTwo = true;
	bDoOnceThree = true;
	bInformationGiven = false;
	bUserCalled = false;
	AtPoITimeElapsed = 0.0f;
	WaitForUserTimeElapsed = 0.0f;
	Malika = Cast<AMalika>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Omar = Cast<AOmar>(UGameplayStatics::GetActorOfClass(GetWorld(), AOmar::StaticClass()));
	Vivian = Cast<AVivian>(UGameplayStatics::GetActorOfClass(GetWorld(), AVivian::StaticClass()));
}



FVector UAtPoI::GetNearestPosition(AVirtualHuman* VH)
{
	float GuardVal = 100000.0f;
	FVector ParticipantLocation = VH->GetActorLocation();
	FVector NearestPosition;

	for (int i = 0; i < Positions.Num(); i++)
	{
		float Delta = (ParticipantLocation - Positions[i]).Size();

		if (Delta < GuardVal && PointsFree[i]) {
			GuardVal = Delta;
			NearestPosition = Positions[i];

		}
	}

	for (int i = 0; i < Positions.Num(); i++)
	{
		if (NearestPosition == Positions[i] && PointsFree[i])
		{
			PointsFree[i] = false;
			return Positions[i];

		}

	}
	return {};

}



FVector UAtPoI::GetNearestPosition(FVector Position)
{
	float GuardVal = 100000.0f;
	FVector NearestPosition;

	for (int i = 0; i < Positions.Num(); i++)
	{
		float Delta = (Position - Positions[i]).Size();

		if (Delta < GuardVal && PointsFree[i]) {
			GuardVal = Delta;
			NearestPosition = Positions[i];

		}
	}

	for (int i = 0; i < Positions.Num(); i++)
	{
		if (NearestPosition == Positions[i] && PointsFree[i])
		{
			PointsFree[i] = false;
			return Positions[i];

		}

	}
	return {};

}



FVector UAtPoI::GetFurthestPosition(AVirtualHuman* VH)
{
	float GuardVal = 1.0f;
	FVector ParticipantLocation = VH->GetActorLocation();
	FVector FurthestPosition;

	for (int i = 0; i < Positions.Num(); i++)
	{
		float Delta = (ParticipantLocation - Positions[i]).Size();

		if (Delta > GuardVal && PointsFree[i]) {
			GuardVal = Delta;
			FurthestPosition = Positions[i];

		}
	}

	for (int i = 0; i < Positions.Num(); i++)
	{
		if (FurthestPosition == Positions[i] && PointsFree[i])
		{
			PointsFree[i] = false;
			return Positions[i];

		}

	}
	return {};

}


void UAtPoI::GuideMoveToPosition()
{
	GuideController = Cast<AGuideAIController>(Guide->VHPawn->GetController());
	GuidePositionLocation = GetFurthestPosition(Guide->VH);
	GuideController->MoveToLocation(GuidePositionLocation, 10.0f, true);
}


bool UAtPoI::CheckUserLocation()
{
	/*
	FVector UserAssignedSlot;

	for (int i = 0; i < Positions.Num(); i++)
	{
		if ((Guide->UserLocation - Positions[i]).Size() < 200.0f)
		{
			UserAssignedSlot = Positions[i];
			PointsFree[i] = false;
			return true;
		}
	}
	return false;
	*/

	if ((Guide->VRPawn->Head->GetComponentLocation() - Guide->CurrentPoI->GetActorLocation()).Size() < 400)
	{
		return true;
	}
	else
	{
		return false;
	}

}


void UAtPoI::GazeModel(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;
	int i;
	float AgentToUserAngle;
	const FVector Offset = FVector(0, 0, Guide->VH->BaseEyeHeight);

	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeState)
		{
		case EGazeState::ToUser:
			Guide->FacialExpressComp->SelectedEmotion = Happiness;
			Guide->GazingComp->GazeToUser();
			GazePhaseDuration = FMath::FRandRange(2.0, 6.0);
			GazeState = EGazeState::ToPoI;
			break;
		case EGazeState::ToPoI:
			Guide->FacialExpressComp->SelectedEmotion = Neutral;
			Guide->GazingComp->GazeTo(Guide->CurrentPoI->GetRefPointGazing());
			GazePhaseDuration = FMath::FRandRange(2.0, 3.0);
			GazeState = EGazeState::ToParticipant;
			break;
		case EGazeState::ToParticipant:
			Guide->FacialExpressComp->SelectedEmotion = Happiness;
			i = FMath::RandRange(0, 2);
			AgentToUserAngle = FVector::DotProduct(Guide->VH->GetActorForwardVector(), Followers[i]->GetActorLocation() - Guide->VH->GetActorLocation());
			AgentToUserAngle = FMath::Acos(AgentToUserAngle);
			AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);
			if (AgentToUserAngle <= PeripheralVisionAngle) {
				Guide->GazingComp->GazeToActor(Followers[i], Offset);
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


void UAtPoI::OnWaitForUser()
{
	//Guide->TurnTo(Guide->UserLocation, 60.0f, 5.0f, 1.0f);
	GazeState = ToUser;
	//Guide->GazingComp->GazeToUser();
	Guide->PrintDebug("[VHGuide]:Hey, please come to the group.\n");
	Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.UserFallsBehind, 0.5f, true);
	PoIState = WaitForUser;
	bUserCalled = true;

}



void UAtPoI::GiveInformation()
{
	bInformationGiven = true;
	Guide->PrintDebug("[VHGuide]:Guide gives information about the PoI\n");
	Guide->SpeechComp->PlayDialogue(Guide->GetPoi()->Dialogue.Monologue, 3.0f, true);
}