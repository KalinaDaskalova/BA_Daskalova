// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "States.h"
#include "VHGuide.h"
#include "UObject/NoExportTypes.h"
#include "TimerManager.h"
#include "Waypoint.h"
#include "ApproachUser.generated.h"

/**
 * 
 */


UCLASS()
class STADT_API UApproachUser : public UObject, public IStates
{
	GENERATED_BODY()

public:
	UApproachUser();
	virtual void Tick(float DeltaTime) override;
	virtual void OnStateChange() override;
	virtual void OnInterruptionBegin() override;
	void OnSpeechCompleted();

	UVHGuide* Guide;
	AWaypoint* NearestWaypoint;

	FTimerHandle TickTimer;
	float TimeElapsed;
	bool bAudioPlayed;
	bool bGetUserAudioPlayed;
	bool bWalkToUser;
	bool bGuideIsSpeaking;
	bool bGetUser;
};
