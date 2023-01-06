// Fill out your copyright notice in the Description page of Project Settings.

#include "ApproachUser.h"
#include "GameFramework/CharacterMovementComponent.h"

UApproachUser::UApproachUser()
{
	Guide = Cast<UVHGuide>(GetOuter());
	if (IsValid(Guide)) Guide->PrintDebug("State ApproachUser created");
}

void UApproachUser::OnStateChange()
{
	//GetWorld()->GetTimerManager().SetTimer(TickTimer, this, &UApproachUser::Tick, 0.5f, true, 0.5f);
	Guide->MovementComp->MaxWalkSpeed = Guide->UserMaxSpeed * 1.20f;
	bAudioPlayed = false;
	bWalkToUser = false;
	bGetUser = false;
	bGetUserAudioPlayed = false;
	bGuideIsSpeaking = false;
	TimeElapsed = 0.0f;
}

void UApproachUser::Tick(float DeltaTime)
{
	//Time to check how long the user is away from the group
	TimeElapsed += DeltaTime;
	FVector ToUser = Guide->VRPawn->Head->GetComponentLocation() - Guide->VHPawn->GetActorLocation();
	float DistanceToUser = ToUser.Size();

	//Stop and play an audio "Hey, over here!"
	if (!bAudioPlayed)
	{
		Guide->MovementComp->StopActiveMovement();
		Guide->GazingComp->GazeToUser();
		//Guide->TurnTo(Guide->VRPawn->Head->GetComponentLocation(), 60.0f, 5.0f, DeltaTime);
		Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.UserFallsBehind, 0.1f, true);
		//Set audio played
		bAudioPlayed = true;
	}
	//If user is coming back to the group and guide is not walking towards user
	if (DistanceToUser <= 500 && TimeElapsed > 5.0f && bAudioPlayed && !bWalkToUser)
	{
		OnSpeechCompleted();
	}
	//If user is getting further away from group, time has elapsed and Audio to get back to group has been played
	if (DistanceToUser > 500 && TimeElapsed > 15.0f && bAudioPlayed && !bWalkToUser)
	{
		Guide->AIController->MoveToLocation(Guide->UserLocation, 150.0);
		bWalkToUser = true;
	}
	if (DistanceToUser < 155.0f && bWalkToUser)
	{
		if (!bGetUserAudioPlayed)
		{
			//"It's a nice city isn't it, but let's move on together to the next attraction"
			Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.UserFallsBehind, 0.1f, true);
			bGetUserAudioPlayed = true;
		}
		if (Guide->SpeechComp->bIsSpeaking)
		{
			bGuideIsSpeaking = true;
		}
		if (bGuideIsSpeaking)
		{
			if (!(Guide->SpeechComp->bIsSpeaking))
			{
				bGetUser = true;
			}
		}
	}
	if (bGetUser)
	{
		Guide->AIController->MoveToLocation(Guide->LocationBeforeInterrupt);
	}
	if ((Guide->VHPawn->GetActorLocation() - Guide->LocationBeforeInterrupt).Size() < 50.0f && bGetUser)
	{
		OnSpeechCompleted();
	}
}


void UApproachUser::OnSpeechCompleted()
{
	Guide->MovementComp->MaxWalkSpeed = 100;
	Guide->StateBeforeInterruption->OnInterruptionEnd();
	//On interruption end is called when guide is back with the group
	Guide->State = Guide->StateBeforeInterruption;
}


//This will not be used probably
void UApproachUser::OnInterruptionBegin()
{
	Guide->PrintDebug("Approach User: On Interruption Begin");
}