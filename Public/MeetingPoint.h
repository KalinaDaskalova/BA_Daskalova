// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <VirtualHumanAIController.h>
#include "GuideAIController.h"
#include <Runtime/AIModule/Classes/Navigation/PathFollowingComponent.h>
#include "VHGazing.h"
#include "AIController.h"
#include "VirtualHuman.h"
#include "VHGuide.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Vivian.h"
#include "Malika.h"
#include "Omar.h"
#include "VHAnimInstance.h"
#include "VirtualHumanAIController.h"
#include "VHSpeech.h"
#include "States.h"
#include "SocialGroups/SocialGroup.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Pawn/VirtualRealityPawn.h"
#include "MeetingPoint.generated.h"


UENUM()
enum EAtMeetingPoint
{
	Conversation,
	UserInGroup,
	GiveInformation,
};


UCLASS()
class STADT_API UMeetingPoint : public UObject, public IStates
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UMeetingPoint();

	void Tick(float DeltaTime) override;
	virtual void GazeModel(float DeltaTime) override;
	virtual void OnStateChange() override;
	//virtual void OnInterruptionBegin() override;
	//virtual void OnInterruptionEnd() override;
	ASocialGroup* SG;


	bool UserOrientedToGroup();


	UVHGuide* Guide;
	AMalika* Malika;
	AVivian* Vivian;
	AOmar* Omar;

	bool bdoOnce;
	bool bdoOnceTwo;
	bool bdoOnceThree;
	bool bDoOnceFour;
	bool bGuideIsSpeaking;


	//Waving animation
	FTimerHandle PlayWavingTimer;
	FTimerHandle PlayVivianDialogue1;
	FTimerHandle PlayVivianDialogue2;
	FTimerHandle PlayVivianDialogue3;
	FTimerHandle PlayOmarDialogue1;
	FTimerHandle PlayOmarDialogue2;
	FTimerHandle PlayOmarDialogue3;
	FTimerHandle PlayOmarDialogue4;
	FTimerHandle PlayOmarDialogue5;
	FTimerHandle PlayMalikaDialogue1;
	FTimerHandle PlayMalikaDialogue2;
	FTimerHandle PlayMalikaDialogue3;
	FTimerHandle PlayMalikaDialogue4;
	FTimerHandle PlayMalikaDialogue5;
	FTimerDelegate PlayMontageDelegate;
	void PlayWaving(UAnimMontage* Montage);
	void VivianDialogue1();
	void VivianDialogue2();
	void VivianDialogue3();
	void MalikaDialogue1();
	void MalikaDialogue2();
	void MalikaDialogue3();
	void MalikaDialogue4();
	void MalikaDialogue5();
	void OmarDialogue1();
	void OmarDialogue2();
	void OmarDialogue3();
	void OmarDialogue4();
	void OmarDialogue5();

	//Conversation animation
	FTimerHandle PlayConversationTimer;
	FTimerDelegate PlayConversationDelegate;
	void PlayConversation();

	UPROPERTY()
		TEnumAsByte<EAtMeetingPoint> MPState;

};