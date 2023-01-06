// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VirtualHuman.h"
#include "AITypes.h"
#include "AIController.h"
#include "PoI.h"
#include "GuideAIController.h"
#include "VHMovement.h"
#include "VHAnimInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TextRenderComponent.h"
#include "SocialGroups/VHSocialGroups.h"
#include "VHGazing.h"
#include "VHFacialExpressions.h"
#include "VHPointing.h"
#include "VHSpeech.h"
#include "Helper/VHFaceAnimation.h"
#include "SoundSource/VAAudiofileSourceComponent.h"
#include "VHGuide.generated.h"

/**
 *
 */

class UMeetingPoint;
class UAtPoI;
class UToPoI;
class UApproachUser;
class UGoodbye;
class IStates;
class AMalika;
class AOmar;
class AVivian;




UENUM()
enum EUserQuestions
{
	QestionOne,
	QuestionTwo,
	QuestionThree,
};



USTRUCT()
struct FGuideDialogues
{
	GENERATED_BODY()

	FDialogueUtterance Noticed;
	FDialogueUtterance Greeting;
	FDialogueUtterance UserFallsBehind;
	FDialogueUtterance Goodbye;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STADT_API UVHGuide : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVHGuide();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PrintDebug(FString Msg);
	void PrintError(FString Msg);

	UPROPERTY(EditAnywhere, Category = "Guide")
		float PersonalDistance = 120.0f;

	FVector UserLocation;
	FVector LocationBeforeInterrupt;
	float GuideSpeed;
	float GuideToUserAngle;
	TArray<AVirtualHuman*> Followers;
	AActor* ParticipantActor;
	AVirtualHuman* Participant;
	FVector GetProximityForce();


	//Animation and Speech 
	FGuideDialogues GuideDialogues;
	void SetGuideDialogues();
	TArray<UAnimMontage*> LoadAnimationMontages() const;
	UPROPERTY()
		TArray<UAnimMontage*> Montages;
	void PreLoadFaceAnimationFiles();

	ACharacter* GuideVH;
	

	UFUNCTION()
		void ChangeState(TScriptInterface<IStates> NewState);

	UPROPERTY()
		TScriptInterface<IStates> State;

	UPROPERTY()
		float UserMaxSpeed;

	//<<<<PointsOfInterest>>>>
	UPROPERTY(EditAnywhere, Category = "Guide")
		TArray<APoI*> PointsOfInterest;
	int CurrentPoIIterator;
	APoI* GetPoi() const
	{
		return CurrentPoI;
	}
	APoI* CurrentPoI;
	void NextPoI();
	void SetPoi(APoI* Poi);

	//<<<<Initializing parameters>>>>
	UCharacterMovementComponent* MovementComp;
	AVirtualRealityPawn* VRPawn;
	UPROPERTY()
		TScriptInterface<IStates> StateBeforeInterruption;
	UPROPERTY()
		UAtPoI* AtPoI;
	UPROPERTY()
		UToPoI* ToPoI;
	UPROPERTY()
		UApproachUser* ApproachUser;
	UPROPERTY()
		UMeetingPoint* MeetingPoint;
	UPROPERTY()
		UGoodbye* Goodbye;
	UPROPERTY()
		APawn* VHPawn;
	UPROPERTY()
		AVirtualHuman* VH;
	UPROPERTY()
		AGuideAIController* AIController;
	UPROPERTY()
		UVHAnimInstance* FaceAnimationInstance;
	UPROPERTY()
		UVHAnimInstance* BodyAnimationInstance;
	UPROPERTY()
		UVHPointing* PointingComp;
	UPROPERTY()
		UVHGazing* GazingComp;
	UPROPERTY()
		UVHSpeech* SpeechComp;
	UPROPERTY()
		UVHFaceAnimation* FaceAnimComp;
	UPROPERTY()
		UVHFacialExpressions* FacialExpressComp;
	UPROPERTY()
		UVAAudiofileSourceComponent* AudioComp;
	TArray<AVirtualHuman*> Participants;
	TArray<AVirtualHuman*> GetParticipants();


	//<<<<<<<<<<<UserQuestions>>>>>>>>>>>>>
	TEnumAsByte<EUserQuestions> UserQuestions;
	UPROPERTY()
		UTextRenderComponent* QuestionOne;
	UPROPERTY()
		UTextRenderComponent* QuestionTwo;
	UPROPERTY()
		UTextRenderComponent* QuestionThree;
	bool bUserCanAskQuestions;
	void DisableUserInput();
	void AllowUserInput();
	
	//<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>



	void TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime);
	float ZValue;
	bool bDesktopMode;
	bool bTurnToUser;
	bool bGreetUser;
	bool bLookAtUser;
	float CurrentYaw;
	float PeripheralVisionAngle = 180.0f;
	float GazeTimeElapsed;
	float GreetTimeElapsed;
	float UserWalkingNextToGuide;
	void MoveToWaypoint();
	void SetSpeed(AVirtualHuman* VirtualH, float Speed);
	UPROPERTY(EditAnywhere, Category = "CurrentWaypoint")
		int CurrentWaypoint;
	TArray<AActor*> Waypoints;
	bool UserOrientedToGuide();
	bool Init();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	FAIRequestID CurrentRequestID;

};
