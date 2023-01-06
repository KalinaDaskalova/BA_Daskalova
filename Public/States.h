// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VHGuide.h"
#include "States.generated.h"


UENUM()
enum EGazeState
{
	ToUser	UMETA(DisplayName = "To user"),
	ToPoI	UMETA(DisplayName = "To point of interest"),
	ToParticipant UMETA(DisplayName = "To participant"),
	Ahead	UMETA(DisplayName = "Ahead"),
	AtRandom UMETA(DisplayName = "Random"),
};

UENUM()
enum EGazeStatePoI
{
	GazeToUser	UMETA(DisplayName = "To user"),
	GazeToPoI	UMETA(DisplayName = "To point of interest"),
	GazeToParticipant UMETA(DisplayName = "To participant"),
	GazeToGuide UMETA(DisplayName = "ToGuide"),
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UStates : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STADT_API IStates
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:
	virtual void Tick(float DeltaTime);
	virtual void OnStateChange();
	virtual void OnInterruptionBegin();
	virtual void OnInterruptionEnd();
	virtual void GazeModel(float DeltaTime);

	float GazeTimeElapsed;
	float GazePhaseDuration;
	TEnumAsByte<EGazeState> GazeState;
	TEnumAsByte<EGazeStatePoI> GazeStatePoI;

	UVHGuide* Guide;

};


