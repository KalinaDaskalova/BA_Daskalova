// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VirtualHuman.h"
#include "Malika.h"
#include "Omar.h"
#include "Vivian.h"
#include "TimerManager.h"
#include "PoI.h"
#include "ApproachUser.h"
#include "VHGuide.h"
#include "States.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include <VirtualHumanAIController.h>
#include <map>
#include "ToPoI.generated.h"

/**
 *
 */

UENUM()
enum EToPoi
{
	ApproachMon,
	StandingStill,
	CalculatePositions,
	FollowPositions,
};


UENUM()
enum EFormations
{
	Grid		UMETA(DisplayName = "Grid"),
	River		UMETA(DisplayName = "River"),
};

UENUM()
enum EToFormation
{
	ToGrid		UMETA(DisplayName = "ToGrid"),
	ToRiver     UMETA(DisplayName = "ToRiver"),
};

UCLASS()
class STADT_API UToPoI : public UObject, public IStates
{
	GENERATED_BODY()

public:

	UToPoI();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnStateChange() override;
	virtual void OnInterruptionBegin() override;
	virtual void OnInterruptionEnd() override;
	virtual void GazeModel(float DeltaTime) override;

	UVHGuide* Guide;

	//<<<<<<Gazing>>>>>>
	float UserInSightTimeElapsed;
	float UserInSightPhaseDuration;
	bool bUserWalkedInSight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToPoI")
		AActor* Leader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToPoI")
		TArray <AVirtualHuman*> Followers;
	AVirtualHuman* Participant;
	AMalika* Malika;
	//TMap<AMalika*, FVector*> MalikaAssignedSlot;
	AOmar* Omar;
	//TMap<AOmar*, FVector*> OmarAssignedSlot;
	AVivian* Vivian;
	//TMap<AVivian*, FVector*> VivianAssignedSlot;
	TArray<FVector*> PositionsPointer;
	TArray<FVector> Positions;
	TArray<FVector> InterpPositions;
	FVector* UserAssignedSlot;

	FAIRequestID CurrentRequestID;

	UPROPERTY()
		TEnumAsByte<EToPoi> ToPoIState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToPoI")
		TEnumAsByte<EFormations> Formations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ToPoI")
		TEnumAsByte<EToFormation> ToFormation;

	UPROPERTY()
		AVirtualRealityPawn* VRPawn;

	AVirtualHumanAIController* AIController;
	AActor* PoIActor;
	APoI* PoI;
	UPROPERTY()
		UAtPoI* AtPoI;
	UPROPERTY()
		UToPoI* ToPoI;
	UPROPERTY()
		UApproachUser* ApproachUser;



	UCharacterMovementComponent* MovementComp;

	FVector MalikaNearestFreePosition();
	FVector OmarNearestFreePosition();
	FVector VivianNearestFreePosition();

	bool CheckUserWhenInterpolate();
	FVector GetCurrentUserLocation();
	FVector UserLocation;
	FVector GuideLocation;
	TMap<AVirtualHuman*, FVector*> AssignedSlots;
	float LeftHitDistance;
	float RightHitDistance;
	float WaitTimeElapsed;
	float HitResultTime;
	float NoHitTime;
	float PointsNotAssigned;

	bool bSetPositions;
	bool bPointerArrayNotSet;
	bool bPositionsNotSet;
	bool bGuideAtPoI;
	bool bGuideIsMoving;
	bool bParametersNotSet;
	bool bFormationChanged;
	bool bGroupStartWalking;
	bool bDoOnce;
	bool bDoOnceTwo;
	bool bDoOnceThree;
	bool bChangeToRiver;
	bool bChangeFromRiver;

	void CalculatePoints();
	void FollowGuide();
	void MoveToLocation();
	void MoveToLocation(AVirtualHuman* Participant);
	void PerformRaycast(float DeltaTime);
	void InterpolatePoints();
	void SetPointerArray();
	bool CheckUserLocation();
	bool GetUserLocation();
	void CheckDistanceToUser();
	bool GuideAtPoI();
	void GroupStartWalking();
	void ActivateSetPositions();
	//Checks distance to desired point
	void CheckDistanceToPoint(AVirtualHuman* VH, FVector DesiredLocation, UCharacterMovementComponent* MovementComp);
	bool IsMoving(AActor* VH);
	bool IsMoving(APawn* Pawn);

	float CalculateAngle(FVector VectorA, FVector VectorB);
	float CalculateDistance(float HitDistance, FVector ForwardVector, FVector SideVector);

	TArray<bool> PointsFree;
	void Formation();
	void ChangeFormation();


	//Timers
	void UnPauseTimer(FTimerHandle TimerHandle);
	void CalculatePointsTimer(float FirstDelay);
	void MalikaStartTick(float FirstDelay);
	void OmarStartTick(float FirstDelay);
	void VivianStartTick(float FirstDelay);
	void MalikaEnableTick();
	void OmarEnableTick();
	void VivianEnableTick();
	FTimerHandle PointsTimer;
	void MoveToLocationTimer(float FirstDelay);
	FTimerHandle ChangeFormationTimer;
	//FTimerHandle MoveTimer;
	FTimerHandle MalikaTick;
	FTimerHandle OmarTick;
	FTimerHandle VivianTick;
};

