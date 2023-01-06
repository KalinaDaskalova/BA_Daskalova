// Fill out your copyright notice in the Description page of Project Settings.


#include "ToPoI.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "VHGuide.h"
#include "Engine.h"
#include "ApproachUser.h"
#include "Goodbye.h"
#include "AtPoI.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/Character.h"


// Sets default values
UToPoI::UToPoI()
{
	bDoOnceThree = true;
	bChangeToRiver = false;
	bChangeFromRiver = false;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
}



void UToPoI::OnStateChange()
{
	PointsFree.Empty();
	Positions.Empty();
	PositionsPointer.Empty();
	AssignedSlots.Empty();
	for (int i = 0; i < 4; i++)
	{
		PointsFree.Add(true);
	}
	if (bChangeToRiver)
	{
		Formations = River;
		bChangeToRiver = false;
	}
	else if (!bChangeToRiver)
	{
		Formations = Grid;
	}
	if (!bChangeFromRiver)
	{
		ToPoIState = ApproachMon;
		bChangeFromRiver = false;
	}
	else if (bChangeFromRiver)
	{
		ToPoIState = StandingStill;
	}
	bPointerArrayNotSet = true;
	PointsNotAssigned = 0.0f;
	bPositionsNotSet = true;
	bGuideAtPoI = true;
	bGuideIsMoving = false;
	bParametersNotSet = true;
	bSetPositions = false;
	bFormationChanged = true;
	bGroupStartWalking = false;
	bDoOnce = true;
	bDoOnceTwo = true;
	UserInSightTimeElapsed = 0;
	UserInSightPhaseDuration = 0;
	bUserWalkedInSight = false;
	Guide = Cast<UVHGuide>(GetOuter());
	VRPawn = Cast<AVirtualRealityPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	Malika = Cast<AMalika>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Omar = Cast<AOmar>(UGameplayStatics::GetActorOfClass(GetWorld(), AOmar::StaticClass()));
	Vivian = Cast<AVivian>(UGameplayStatics::GetActorOfClass(GetWorld(), AVivian::StaticClass()));
	//Participant = Cast<AVHParticipant>(UGameplayStatics::GetActorOfClass(GetWorld(), AVHParticipant::StaticClass()));
	Leader = Guide->GetOwner();
	//UGameplayStatics::GetActorOfClass(GetWorld(), UVHGuide::StaticClass());
	Followers = Guide->Followers;
	if (bDoOnceThree)
	{
		Guide->MoveToWaypoint();
		bDoOnceThree = false;
	}
	
}


void UToPoI::OnInterruptionBegin()
{
	GetWorld()->GetTimerManager().PauseTimer(PointsTimer);
	//Malika->SetActorTickEnabled(false);
	//Vivian->SetActorTickEnabled(false);
	//Omar->SetActorTickEnabled(false);
	Guide->ChangeState(Guide->ApproachUser);
}


void UToPoI::OnInterruptionEnd()
{

	Guide->MoveToWaypoint();
	GetWorld()->GetTimerManager().UnPauseTimer(PointsTimer);
	//Malika->SetActorTickEnabled(true);
	//Vivian->SetActorTickEnabled(true);
	//Omar->SetActorTickEnabled(true);
	Guide->PrintDebug("Guide: On Interruption End");

}


// Called every frame
void UToPoI::Tick(float DeltaTime)
{
	WaitTimeElapsed += DeltaTime;
	PointsNotAssigned += DeltaTime;
	switch (ToPoIState)
	{
	case(ApproachMon):
		// Play approach dia if we are not a child
		if (Guide->GetPoi()->Dialogue.ApproachMon.Audiofile != "" && Guide->GetPoi()->Dialogue.ApproachMon.FaceAnimFile != "")
		{
			Guide->SpeechComp->PlayDialogue(Guide->GetPoi()->Dialogue.ApproachMon, 1.0f, true);
			ToPoIState = StandingStill;
		}
		else
		{
			ToPoIState = StandingStill;
		}
		break;
	case(StandingStill):
		//First we check if the guide is moving
		if (IsMoving(Leader))
		{
			bGuideIsMoving = true;
			ToPoIState = CalculatePositions;
		}
		break;
	case(CalculatePositions):
		//Track User Location
		UserLocation = GetCurrentUserLocation();
		if (bDoOnce)
		{
			CalculatePointsTimer(0.5f);
			bDoOnce = false;
		}
		if (bSetPositions)
		{
			SetPointerArray();
			bSetPositions = false;
		}
		if (IsMoving(VRPawn) || WaitTimeElapsed > 3.0f)
		{
			FollowGuide();
			if (CheckUserLocation())
			{
				MalikaNearestFreePosition();
				Malika->SetActorTickEnabled(true);
				VivianNearestFreePosition();
				Vivian->SetActorTickEnabled(true);
				OmarNearestFreePosition();
				Omar->SetActorTickEnabled(true);
				ToPoIState = FollowPositions;
			}
			else if (PointsNotAssigned > 5.0f)
			{
				GetUserLocation();
				MalikaNearestFreePosition();
				Malika->SetActorTickEnabled(true);
				VivianNearestFreePosition();
				Vivian->SetActorTickEnabled(true);
				OmarNearestFreePosition();
				Omar->SetActorTickEnabled(true);
				ToPoIState = FollowPositions;
			}
		}
		break;
	case(FollowPositions):
		PerformRaycast(DeltaTime);
		GuideLocation = Leader->GetActorLocation();
		if (GuideAtPoI()) {
			//Malika->SetActorTickEnabled(false);
			//Vivian->SetActorTickEnabled(false);
			//Omar->SetActorTickEnabled(false);
			GetWorld()->GetTimerManager().ClearTimer(PointsTimer);
			Guide->ChangeState(Guide->AtPoI);
		}
		break;
	}
}


void UToPoI::Formation()
{
	switch (Formations)
	{
	case Grid:
		if (bPositionsNotSet)
		{

			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(-65.0f));
			DrawDebugSphere(GetWorld(), Positions[0], 20.0f, 30.0f, FColor::Green, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(65.0f));
			DrawDebugSphere(GetWorld(), Positions[1], 20.0f, 30.0f, FColor::Red, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-260.0f) + (Leader->GetActorRightVector())*(-65.0f));
			DrawDebugSphere(GetWorld(), Positions[2], 20.0f, 30.0f, FColor::Blue, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-260.6f) + (Leader->GetActorRightVector())*(65.0f));
			DrawDebugSphere(GetWorld(), Positions[3], 20.0f, 30.0f, FColor::Yellow, false, 0.5f);

			bPositionsNotSet = false;
			bSetPositions = true;
		}

		else {

			Positions[0] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(-65.0f));
			DrawDebugSphere(GetWorld(), Positions[0], 20.0f, 30.0f, FColor::Green, false, 0.5f);
			Positions[1] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(65.0f));
			DrawDebugSphere(GetWorld(), Positions[1], 20.0f, 30.0f, FColor::Red, false, 0.5f);
			Positions[2] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-260.0f) + (Leader->GetActorRightVector())*(-65.0f));
			DrawDebugSphere(GetWorld(), Positions[2], 20.0f, 30.0f, FColor::Blue, false, 0.5f);
			Positions[3] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-260.0f) + (Leader->GetActorRightVector())*(65.0f));
			DrawDebugSphere(GetWorld(), Positions[3], 20.0f, 30.0f, FColor::Yellow, false, 0.5f);

		}
		break;

	case River:
		if (bPositionsNotSet)
		{
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[0], 20.0f, 30.0f, FColor::Green, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-230.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[1], 20.0f, 30.0f, FColor::Red, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-330.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[2], 20.0f, 30.0f, FColor::Blue, false, 0.5f);
			Positions.Add(Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-430.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[3], 20.0f, 30.0f, FColor::Yellow, false, 0.5f);

			bPositionsNotSet = false;
			bSetPositions = true;
		}
		else {

			Positions[0] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-130.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[0], 20.0f, 30.0f, FColor::Green, false, 0.5f);
			Positions[1] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-230.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[1], 20.0f, 30.0f, FColor::Red, false, 0.5f);
			Positions[2] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-330.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[2], 20.0f, 30.0f, FColor::Blue, false, 0.5f);
			Positions[3] = (Leader->GetRootComponent()->GetComponentLocation() + (Leader->GetActorForwardVector())*(-430.0f) + (Leader->GetActorRightVector())*(0.0f));
			DrawDebugSphere(GetWorld(), Positions[3], 20.0f, 30.0f, FColor::Yellow, false, 0.5f);

		}

		break;
	}


}


FVector UToPoI::GetCurrentUserLocation()
{
	return VRPawn->Head->GetComponentLocation();
}

/***********Timers************/
void UToPoI::CalculatePointsTimer(float FirstDelay)
{
	GetWorld()->GetTimerManager().SetTimer(PointsTimer, this, &UToPoI::Formation, 0.3f, true, FirstDelay);
}

void UToPoI::MalikaStartTick(float FirstDelay)
{
	GetWorld()->GetTimerManager().SetTimer(MalikaTick, this, &UToPoI::MalikaEnableTick, 4.0f, false, FirstDelay);
}

void UToPoI::OmarStartTick(float FirstDelay)
{
	GetWorld()->GetTimerManager().SetTimer(OmarTick, this, &UToPoI::OmarEnableTick, 4.0f, false, FirstDelay);
}

void UToPoI::VivianStartTick(float FirstDelay)
{
	GetWorld()->GetTimerManager().SetTimer(VivianTick, this, &UToPoI::VivianEnableTick, 4.0f, false, FirstDelay);
}

void UToPoI::MalikaEnableTick()
{
	Malika->SetActorTickEnabled(true);
}

void UToPoI::OmarEnableTick()
{
	Omar->SetActorTickEnabled(true);
}

void UToPoI::VivianEnableTick()
{
	Vivian->SetActorTickEnabled(true);
}

void UToPoI::FollowGuide()
{
	for (int i = 0; i < Followers.Num(); i++)
	{

		MovementComp = Cast<UCharacterMovementComponent>(Followers[i]->GetMovementComponent());
		AIController = Cast<AVirtualHumanAIController>(Followers[i]->GetController());
		MovementComp->MaxWalkSpeed = 100;
		CurrentRequestID = AIController->MoveToLocation(Leader->GetActorLocation(), 70.0f, false, true, true);

	}
}
/*
void UToPoI::MoveToLocation()
{

	for (int i = 0; i < Followers.Num(); i++)
	{

		MovementComp = Cast<UCharacterMovementComponent>(Followers[i]->GetMovementComponent());
		AIController = Cast<AVirtualHumanAIController>(Followers[i]->GetController());
		FVector DesiredLocation = **(AssignedSlots.Find(Followers[i]));
		CheckDistanceToPoint(Followers[i], DesiredLocation, MovementComp);
		CurrentRequestID = AIController->MoveToLocation(DesiredLocation, 1.0f, false, true, true);

	}

}
*/

/*
void UToPoI::MoveToLocation(AVirtualHuman* AParticipant)
{

	MovementComp = Cast<UCharacterMovementComponent>(AParticipant->GetMovementComponent());
	AIController = Cast<AVirtualHumanAIController>(AParticipant->GetController());
	FVector DesiredLocation = **(AssignedSlots.Find(AParticipant));
	CheckDistanceToPoint(AParticipant, DesiredLocation, MovementComp);
	CurrentRequestID = AIController->MoveToLocation(DesiredLocation, 1.0f, false, true, true);

}
*/

FVector UToPoI::MalikaNearestFreePosition()
{

	float GuardVal = 10000.0f;
	FVector ParticipantLocation = Malika->GetActorLocation();
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
			Malika->AssignedSlot = PositionsPointer[i];
			return Positions[i];
		}
	}
	return {};
}


FVector UToPoI::OmarNearestFreePosition()
{

	float GuardVal = 10000.0f;
	FVector ParticipantLocation = Omar->GetActorLocation();
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
			Omar->AssignedSlot = PositionsPointer[i];
			return Positions[i];

		}

	}
	return {};
}


FVector UToPoI::VivianNearestFreePosition()
{

	float GuardVal = 10000.0f;
	FVector ParticipantLocation = Vivian->GetActorLocation();
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
			Vivian->AssignedSlot = PositionsPointer[i];
			return Positions[i];

		}

	}
	return {};
}

/*
FVector UToPoI::FindNearestFreePosition(AVirtualHuman* AParticipant)
{

	float GuardVal = 10000.0f;
	FVector ParticipantLocation = AParticipant->GetActorLocation();
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
			AssignedSlots.Add(AParticipant, PositionsPointer[i]);
			return Positions[i];

		}

	}
	return {};
}
*/


void UToPoI::PerformRaycast(float DeltaTime)
{
	NoHitTime += DeltaTime;
	FHitResult HitResult;
	FVector StartTrace = (Leader->GetRootComponent())->GetComponentLocation();
	TArray<FVector> Vectors;
	Vectors.Add(Leader->GetActorForwardVector()); //Leader's forward vector
	Vectors.Add(Leader->GetActorRightVector()); //Leader's right vector
	Vectors.Add((Leader->GetActorRightVector())*(-1)); //Leader's left vector
	Vectors.Add((Leader->GetActorForwardVector()) + (Leader->GetActorRightVector())); //Leader's interpolated Vector between forward one and right one
	Vectors.Add((Leader->GetActorForwardVector()) + ((Leader->GetActorRightVector())*(-1))); //Leader's interpolated Vector between forward one and left one
	


	TArray<FVector> EndTraces;
	for (int i = 0; i < Vectors.Num(); i++)
	{
		EndTraces.Add(((Vectors[i] * 2000.0f) + StartTrace));
	}

	FCollisionQueryParams* TraceParams = new FCollisionQueryParams();

	for (int i = 0; i < Vectors.Num(); i++)
	{
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTraces[i], ECC_Visibility, *TraceParams))
		{
			DrawDebugLine(GetWorld(), StartTrace, EndTraces[i], FColor(255, 0, 0), false, 0.5f);
			if (i == 3)
			{

				RightHitDistance = CalculateDistance(HitResult.Distance, Vectors[0], Vectors[3]);

			}
			if (i == 4)
			{
				LeftHitDistance = CalculateDistance(HitResult.Distance, Vectors[0], Vectors[4]);

			}
			
		}
	}
		if (RightHitDistance <= 150.0f && Formations == Grid && bFormationChanged == true && HitResult.Actor != VRPawn && LeftHitDistance <= 150.0f)
		{
			NoHitTime = 0.0f;
			ToFormation = ToRiver;
			InterpolatePoints();
			//GetWorld()->GetTimerManager().ClearTimer(PointsTimer);
			//bChangeToRiver = true;
			//OnStateChange();
			bFormationChanged = false;
		}

		//Distance is more than 1,50 m and we were in river formation then switch to grid
		if (RightHitDistance > 150.0f && NoHitTime > 5.0f && Formations == River && bFormationChanged == false && LeftHitDistance > 150.0f) 
		{
			bFormationChanged = true;
			GetWorld()->GetTimerManager().SetTimer(ChangeFormationTimer, this, &UToPoI::ChangeFormation, 0.5f, false, 0.5f);
		}
				
}




void UToPoI::InterpolatePoints()
{

	switch (ToFormation)
	{
	case(ToRiver):
		int i;
		UserAssignedSlot = nullptr;
		if (CheckUserWhenInterpolate())
		{
			//Check which position the user is nearest to
			//ChechUserLocation->check which point the user is closest to
			if (*UserAssignedSlot == Positions[0]) {
				Malika->SetActorTickEnabled(false);
				Vivian->SetActorTickEnabled(false);
				Omar->SetActorTickEnabled(false);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("User is at point 0")));
				if (Malika->AssignedSlot == PositionsPointer[1])
				{
					//Play Malika->GoAhead;
					//When Malika stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(4.0f);
					VivianStartTick(5.0f);
					OmarStartTick(5.0f);

				}
				if (Omar->AssignedSlot == PositionsPointer[1])
				{
					//Play Omar->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(5.0f);
					VivianStartTick(5.0f);
					OmarStartTick(4.0f);
				}
				if (Vivian->AssignedSlot == PositionsPointer[1])
				{
					//Play Vivian->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(5.0f);
					VivianStartTick(4.0f);
					OmarStartTick(5.0f);
				}
			}
			if (*UserAssignedSlot == Positions[1]) {
				Malika->SetActorTickEnabled(false);
				Vivian->SetActorTickEnabled(false);
				Omar->SetActorTickEnabled(false);
				UserAssignedSlot = PositionsPointer[0];
				if (Malika->AssignedSlot == PositionsPointer[0])
				{
					Malika->AssignedSlot = PositionsPointer[1];
					//Play Malika->GoAhead;
					//When Malika stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(4.0f);
					VivianStartTick(5.0f);
					OmarStartTick(5.0f);
				}
				if (Omar->AssignedSlot == PositionsPointer[0])
				{
					Omar->AssignedSlot = PositionsPointer[1];
					//Play Omar->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(5.0f);
					VivianStartTick(5.0f);
					OmarStartTick(4.0f);
				}
				if (Vivian->AssignedSlot == PositionsPointer[0])
				{
					Vivian->AssignedSlot = PositionsPointer[1];
					//Play Vivian->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.GoAhead, 0.1f, true);
					}
					MalikaStartTick(5.0f);
					VivianStartTick(4.0f);
					OmarStartTick(5.0f);
				}


			}
			if (*UserAssignedSlot == Positions[2]) {
				if (Malika->AssignedSlot == PositionsPointer[3])
				{
					Malika->SetActorTickEnabled(false);
					//Play Malika->GoAhead;
					//When Malika stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.GoAhead, 0.1f, true);
					}
					//Probably set a timer to start ticking again
					MalikaStartTick(4.0f);
				}
				if (Omar->AssignedSlot == PositionsPointer[3])
				{
					Omar->SetActorTickEnabled(false);
					//Play Omar->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.GoAhead, 0.1f, true);
					}
					OmarStartTick(4.0f);
				}
				if (Vivian->AssignedSlot == PositionsPointer[3])
				{
					Vivian->SetActorTickEnabled(false);
					//Play Vivian->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.GoAhead, 0.1f, true);
					}
					VivianStartTick(4.0f);
				}
			}
			if (*UserAssignedSlot == Positions[3]) {
				UserAssignedSlot = PositionsPointer[2];
				if (Malika->AssignedSlot == PositionsPointer[2])
				{
					Malika->SetActorTickEnabled(false);
					Malika->AssignedSlot = PositionsPointer[3];
					//Play Malika->GoAhead;
					//When Malika stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Malika->GazingComp->GazeToUser();
						Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.GoAhead, 0.1f, true);
					}

					MalikaStartTick(4.0f);
				}
				if (Omar->AssignedSlot == PositionsPointer[2])
				{
					Omar->SetActorTickEnabled(false);
					Omar->AssignedSlot = PositionsPointer[3];
					//Play Omar->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Omar->GazingComp->GazeToUser();
						Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.GoAhead, 0.1f, true);
					}
					OmarStartTick(4.0f);
				}
				if (Vivian->AssignedSlot == PositionsPointer[2])
				{
					Vivian->SetActorTickEnabled(false);
					Vivian->AssignedSlot = PositionsPointer[3];
					//Play Vivian->GoAhead;
					//When Omar stops talking start ticking for all participants
					i = FMath::RandRange(0, 1);
					if (i == 0)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.AfterYou, 0.1f, true);
					}
					else if (i == 1)
					{
						Vivian->GazingComp->GazeToUser();
						Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.GoAhead, 0.1f, true);
					}
					VivianStartTick(4.0f);
				}
			}
		}
		Formations = River;
		break;
	}
}



//Change Formation to Grid
void UToPoI::ChangeFormation()
{
	GetWorld()->GetTimerManager().ClearTimer(PointsTimer);
	bChangeFromRiver = true;
	OnStateChange();
}

//Unpause Timers
void UToPoI::UnPauseTimer(FTimerHandle TimerHandle)
{
	GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
}

//Calculate the angle between two vectors
float UToPoI::CalculateAngle(FVector VectorA, FVector VectorB)
{
	float Angle = UKismetMathLibrary::Acos((FVector::DotProduct(VectorA, VectorB)) / (VectorA.Size()*VectorB.Size()));
	return Angle;
}

//calculate the distance between the forward vector of the leader and the nearest object found by ray casting
float UToPoI::CalculateDistance(float HitDistance, FVector ForwardVector, FVector SideVector)
{
	float ForwardVectorDistance = (HitDistance)*(CalculateAngle(ForwardVector, SideVector)); //Calculating distance where the forward vector meets the side vector when an object is hit
	float Distance = UKismetMathLibrary::Sqrt((HitDistance*HitDistance) - (ForwardVectorDistance*ForwardVectorDistance)); //Pythagorean theorem 
	return Distance;
}

//Setting the pointer array that is going to be used to map the VHs to their slots
void UToPoI::SetPointerArray()
{
	if (bPointerArrayNotSet)
	{
		for (int i = 0; i < Positions.Num(); i++)
		{
			PositionsPointer.Add(&Positions[i]);
		}
		bPointerArrayNotSet = false;
	}
}

//Check if a VH is moving
bool UToPoI::IsMoving(AActor* VH)
{
	float Speed = VH->GetVelocity().Size();
	if (Speed != 0)
	{
		return true;
	}
	else {
		return false;
	}
}

//Check if VRPawn/User is moving
bool UToPoI::IsMoving(APawn* Pawn)
{
	float Speed = Pawn->GetVelocity().Size();
	if (Speed != 0)
	{
		return true;

	}
	else {
		return false;
	}

}


//Checks distance to desired position and increases the VH walking speed if VH is too far from the desired position
void UToPoI::CheckDistanceToPoint(AVirtualHuman* VH, FVector DesiredLocation, UCharacterMovementComponent* MovementComponent)
{
	float Distance = (VH->GetActorLocation() - DesiredLocation).Size();
	if (Distance > 150.0f)
	{
		MovementComponent->MaxWalkSpeed = 150;
	}
	else if (Distance > 100.0f) {

		MovementComponent->MaxWalkSpeed = 120;
	}
	else {
		MovementComponent->MaxWalkSpeed = 100;
	}
}


//On begin play check where the user is, if they are near a position of the formation then assign this point to them and form the group around them
bool UToPoI::GetUserLocation()
{

	float GuardVal = 10000.f;

	for (int i = 0; i < PositionsPointer.Num(); i++)
	{
		float Delta = (GetCurrentUserLocation() - *PositionsPointer[i]).Size();

		if (Delta < GuardVal)
		{
			GuardVal = Delta;
			UserAssignedSlot = PositionsPointer[i];

		}
	}

	if (PositionsPointer[0] == UserAssignedSlot)
	{
		PointsFree[0] = false;
		UserAssignedSlot = PositionsPointer[0];
		return true;
	}
	else if (PositionsPointer[1] == UserAssignedSlot)
	{
		PointsFree[1] = false;
		UserAssignedSlot = PositionsPointer[1];
		return true;
	}
	else if (PositionsPointer[2] == UserAssignedSlot)
	{
		PointsFree[2] = false;
		UserAssignedSlot = PositionsPointer[2];
		return true;
	}
	else if (PositionsPointer[3] == UserAssignedSlot)
	{
		PointsFree[3] = false;
		UserAssignedSlot = PositionsPointer[3];
		return true;
	}
	else {
		return false;
	}

}


bool UToPoI::CheckUserLocation()
{

	for (int i = 0; i < PositionsPointer.Num(); i++)
	{
		if ((GetCurrentUserLocation() - *PositionsPointer[i]).Size() < 180.0f)
		{
			UserAssignedSlot = PositionsPointer[i];
			PointsFree[i] = false;
			return true;
		}
	}
	return false;
}


bool UToPoI::CheckUserWhenInterpolate()
{

	float GuardVal = 350.f;

	for (int i = 0; i < PositionsPointer.Num(); i++)
	{
		float Delta = (GetCurrentUserLocation() - *PositionsPointer[i]).Size();

		if (Delta < GuardVal)
		{
			GuardVal = Delta;
			UserAssignedSlot = PositionsPointer[i];

		}
	}

	if (PositionsPointer[0] == UserAssignedSlot)
	{
		PointsFree[0] = false;
		UserAssignedSlot = PositionsPointer[0];
		return true;
	}
	else if (PositionsPointer[1] == UserAssignedSlot)
	{
		PointsFree[1] = false;
		UserAssignedSlot = PositionsPointer[1];
		return true;
	}
	else if (PositionsPointer[2] == UserAssignedSlot)
	{
		PointsFree[2] = false;
		UserAssignedSlot = PositionsPointer[2];
		return true;
	}
	else if (PositionsPointer[3] == UserAssignedSlot)
	{
		PointsFree[3] = false;
		UserAssignedSlot = PositionsPointer[3];
		return true;
	}
	else {
		return false;
	}
}



//Leader checking the distance to user
void UToPoI::CheckDistanceToUser()
{
	if ((GetCurrentUserLocation() - Leader->GetActorLocation()).Size() > 400.0f)
	{
		AIController = Cast<AVirtualHumanAIController>((Cast<AVirtualHuman>(Leader))->GetController());
		//AIController->StopMovement;
	}

}


//Check if guide is near PoI
bool UToPoI::GuideAtPoI()
{
	if ((GuideLocation - Guide->CurrentPoI->GetActorLocation()).Size() < 350.0f && Guide->State != Guide->Goodbye)
	{
		bGuideAtPoI = true;
		return true;
	}
	else {
		bGuideAtPoI = false;
		return false;
	}

}


void UToPoI::GazeModel(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	UserInSightTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;

	//when the user is in the guide's peripheral vision for the first time, gaze at him
	float AgentToUserAngle = FVector::DotProduct(Guide->VH->GetActorForwardVector(), Guide->VRPawn->GetActorLocation() - Guide->VH->GetActorLocation());
	AgentToUserAngle = FMath::Acos(AgentToUserAngle);
	AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);

	if (AgentToUserAngle <= PeripheralVisionAngle && !bUserWalkedInSight && UserInSightTimeElapsed >= UserInSightPhaseDuration)
	{
		// when the user is beside or in front of the agent for the first time, look at him
		Guide->GazingComp->GazeToUser();
		GazeState = EGazeState::Ahead;
		bUserWalkedInSight = true;
	}


	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeState)
		{
		case EGazeState::Ahead:
			Guide->GazingComp->StopGazing();
			GazePhaseDuration = FMath::FRandRange(4.0, 6.0);
			GazeState = EGazeState::Ahead;
			/*
			// only gaze to the user if he is in visible range
			if (AgentToUserAngle <= PeripheralVisionAngle)
			{
				GazeState = EGazeState::ToUser;
			}
			else
			{
				GazeState = EGazeState::Ahead;
			}
			*/
			break;
		case EGazeState::ToUser:
			//should not be called
			//Guide->GazingComp->GazeToUser();
			//Guide->FacialExpressComp->SelectedEmotion = Happiness;
			//GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
			//GazeState = EGazeState::Ahead;
			break;

		case EGazeState::ToPoI:
			//should not be called
			GazeState = EGazeState::Ahead;
			break;
		case EGazeState::AtRandom:
			//should not be called
			GazeState = EGazeState::Ahead;
			break;
		case EGazeState::ToParticipant:
			/*
			int i = FMath::RandRange(0, 2);
			Guide->GazingComp->GazeToActor(Followers[i], FVector(0, 0, 150.0f));
			GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
			GazeState = EGazeState::Ahead;
			*/
			break;

		default:
			//VH_ERROR("[VHGuide::GazeModel] Unknown Gaze state!\n");
			break;
		}
	}
}
