// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VirtualHuman.h"
#include "AIController.h"
#include "AITypes.h"
#include "AIController.h"
#include "Pawn/VirtualRealityPawn.h"
#include "VirtualHuman.h"
#include "VirtualHumanAIController.h"
#include "VHMovement.h"
#include "VHAnimInstance.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SocialGroups/VHSocialGroups.h"
#include "VHPointing.h"
#include "VHGazing.h"
#include "VHFacialExpressions.h"
#include "VivianTriggerBox.h"
#include "States.h"
#include "VHGuide.h"
#include "VHSpeech.h"
#include "Vivian.generated.h"

/**
 * 
 */
USTRUCT()
struct FVivianDialogues
{
	GENERATED_BODY()

	FDialogueUtterance MP1;
	FDialogueUtterance MP2;
	FDialogueUtterance MP3;
	FDialogueUtterance HeyWelcome;
	FDialogueUtterance AfterYou;
	FDialogueUtterance GoAhead;
	FDialogueUtterance OverHere;
	//FDialogueUtterance ItStartedRaining;
	//FDialogueUtterance LikeTheBuildings;

};


UCLASS()
class STADT_API AVivian : public AVirtualHuman, public IStates
{
	GENERATED_BODY()
	
public:

	AVivian();
	virtual void Tick(float DeltaTime) override;
	void SetActorTicks(bool set);
	TArray<AVirtualHuman*> VHs;
	FVector* AssignedSlot;

	bool Init();
	void MoveToLocation(FVector Location, float Speed, float AccepptanceRadius);

	UPROPERTY()
		APawn* VHPawn;
	UPROPERTY()
		AVirtualHuman* VH;
	UPROPERTY()
		UVHGazing* GazingComp;
	UPROPERTY()
		UVHAnimInstance* FaceAnimationInstance;
	UPROPERTY()
		UVHAnimInstance* BodyAnimationInstance;
	UPROPERTY()
		UVHSpeech* SpeechComp;
	UPROPERTY()
		AVirtualHumanAIController* AIController;
	UPROPERTY()
		UVHFacialExpressions* FacialExpressComp;
	UPROPERTY()
		UVHFaceAnimation* FaceAnimComp;
	UCharacterMovementComponent* MovementComp;
	UPROPERTY()
		UVHPointing* PointingComp;
	AVirtualRealityPawn* VRPawn;
	void CheckDistanceToPoint();
	UVHGuide* Guide;


	//<<<<<<Gazing>>>>>>
	float UserInSightTimeElapsed;
	float UserInSightPhaseDuration;
	bool bUserWalkedInSight;
	void GazeModel(float DeltaTime);
	void GazeModelPoI(float DeltaTime);;
	void TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime);
	//TEnumAsByte<EGazeStatePoI> GazeStatePoI;

	//<<<<<<<<<Dialogues>>>>>>>>>
	FVivianDialogues VivianDialogues;
	void SetVivianDialogues();
	void PreLoadFaceAnimationFiles();
	UPROPERTY(EditAnywhere)
		TArray<AVivianTriggerBox*> DialogueTriggers;

	//<<<<TurnTo>>>>
	bool bTurnToUser;
	bool bDoOnce;
	float ZValue;
	float CurrentYaw;

	//Get reference to Guide
	UVHGuide* GetGuide();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;


};
