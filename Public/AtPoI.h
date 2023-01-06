// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VHGuide.h"
#include "Malika.h"
#include "Omar.h"
#include "Vivian.h"
#include "States.h"
#include "VirtualHumanAIController.h"
#include "AtPoI.generated.h"

UENUM()
enum EAtPoi
{
	ApproachingPoI,
	MoveToPoI,
	AtPoi,
	Information,
	WaitForUser,
	Finished,
};

UCLASS()
class STADT_API UAtPoI : public UObject, public IStates
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UAtPoI();
	virtual void Tick(float DeltaTime) override;
	virtual void OnStateChange() override;
	virtual void GazeModel(float DeltaTime) override;
	AMalika* Malika;
	AOmar* Omar;
	AVivian* Vivian;
	FVector MalikaPosition;
	FVector OmarPosition;
	FVector VivianPosition;
	FVector GuidePositionLocation;
	void GiveInformation();
	void GuideMoveToPosition();
	void OnWaitForUser();
	float WaitForUserTimeElapsed;
	float AtPoITimeElapsed;
	bool bDoOnce;
	bool bDoOnceTwo;
	bool bDoOnceThree;
	bool bGuideIsSpeaking;
	bool bUserCalled;
	bool bInformationGiven;
	FVector GetNearestPosition(AVirtualHuman* VH);
	FVector GetFurthestPosition(AVirtualHuman* VH);
	FVector GetNearestPosition(FVector Position);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToPoI")
		TArray <AVirtualHuman*> Followers;
	AVirtualHuman* Participant;
	void GuideMoveToLocation();
	bool CheckUserLocation();
	UPROPERTY()
		AVirtualHumanAIController* AIController;
	UPROPERTY()
		AGuideAIController* GuideController;
	TArray<bool> PointsFree;
	UPROPERTY()
		TEnumAsByte<EAtPoi> PoIState;
	TArray<FVector> Positions;
	TArray<FVector*> FormationPositions;
	AVirtualHuman* GetNearestParticipant(FVector Positions);

	UVHGuide* Guide;

private:
	float AudioPlayTime;
};
