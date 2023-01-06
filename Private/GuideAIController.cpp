// Fill out your copyright notice in the Description page of Project Settings.


#include "GuideAIController.h"
#include "VHGuide.h"
#include "ToPoI.h"
#include "Engine.h"



void AGuideAIController::BeginPlay()
{
	Super::BeginPlay();
	VHGuide = GetGuide();
}


void AGuideAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{

	if (VHGuide && VHGuide->State == VHGuide->ToPoI)
	{
		VHGuide->MoveToWaypoint();
	}
}

UVHGuide* AGuideAIController::GetGuide()
{
	for (TActorIterator<AVirtualHuman> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
	{
		AVirtualHuman* ACharacter = *CharacterItr;
		VHs.Add(ACharacter);
	}

	for (int i = 0; i < VHs.Num(); i++)
	{
		Guide = Cast<UVHGuide>(VHs[i]->GetComponentByClass(UVHGuide::StaticClass()));
		if (Guide)
		{
			return Guide;
		}
	}
	return Guide;
}