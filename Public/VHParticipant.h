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
#include "VHGazing.h"
#include "VHFacialExpressions.h"
#include "States.h"
#include "VHGuide.h"
#include "VHSpeech.h"
#include "VHParticipant.generated.h"



/**
 *
 */

UCLASS()
class STADT_API AVHParticipant : public AVirtualHuman, public IStates
{
	GENERATED_BODY()

public:

	AVHParticipant();
	virtual void Tick(float DeltaTime) override;
	void SetActorTicks(bool set);
	TArray<AVirtualHuman*> VHs;

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
	UCharacterMovementComponent* MovementComp;
	AVirtualRealityPawn* VRPawn;
	UVHGuide* Guide;


	//<<<<<<Gazing>>>>>>
	float UserInSightTimeElapsed;
	float UserInSightPhaseDuration;
	bool bUserWalkedInSight;
	void GazeModel(float DeltaTime);
	void GazeModelPoI(float DeltaTime);;
	void TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime);
	TEnumAsByte<EGazeStatePoI> GazeStatePoI;

	//<<<<TurnTo>>>>
	bool bTurnToUser;
	float ZValue;
	float CurrentYaw;

	//Get reference to Guide
	UVHGuide* GetGuide();
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;



private:


};
