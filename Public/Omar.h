// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VirtualHuman.h"
#include "AIController.h"
#include "AITypes.h"
#include "AIController.h"
#include "VirtualHuman.h"
#include "VirtualHumanAIController.h"
#include "VHMovement.h"
#include "VHAnimInstance.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SocialGroups/VHSocialGroups.h"
#include "Pawn/VirtualRealityPawn.h"
#include "VHPointing.h"
#include "OmarTriggerBox.h"
#include "VHGazing.h"
#include "VHFacialExpressions.h"
#include "States.h"
#include "VHGuide.h"
#include "VHSpeech.h"
#include "Omar.generated.h"

/**
 *
 */


USTRUCT()
struct FOmarDialogues
{
	GENERATED_BODY()

	FDialogueUtterance MP1;
	FDialogueUtterance MP2;
	FDialogueUtterance MP3;
	FDialogueUtterance MP4;
	FDialogueUtterance MP5;
	FDialogueUtterance AfterYou;
	FDialogueUtterance GoAhead;
	FDialogueUtterance OverHere;
	FDialogueUtterance OkayLetsGo;
	
};


UCLASS()
class STADT_API AOmar : public AVirtualHuman, public IStates
{
	GENERATED_BODY()

public:

	AOmar();
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
	UPROPERTY()
		UVHPointing* PointingComp;
	UCharacterMovementComponent* MovementComp;
	AVirtualRealityPawn* VRPawn;
	void CheckDistanceToPoint();
	UVHGuide* Guide;


	//<<<<<<Gazing>>>>>>
	float UserInSightTimeElapsed;
	float UserInSightPhaseDuration;
	bool bUserWalkedInSight;
	void GazeModel(float DeltaTime);
	void GazeModelPoI(float DeltaTime);
	void TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime);
	//TEnumAsByte<EGazeStatePoI> GazeStatePoI;

	//<<<<<<<<<Dialogues>>>>>>>>>
	FOmarDialogues OmarDialogues;
	void SetOmarDialogues();
	void PreLoadFaceAnimationFiles();
	UPROPERTY(EditAnywhere)
		TArray<AOmarTriggerBox*> DialogueTriggers;

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
