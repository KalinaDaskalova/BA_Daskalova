// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VHSpeech.h"
#include "Engine/TriggerBox.h"
#include "VHGuide.h"
#include "VivianTriggerBox.generated.h"

/**
 *
 */

class AVivian;

UCLASS()
class STADT_API AVivianTriggerBox : public ATriggerBox
{
	GENERATED_BODY()


public:

	AVivianTriggerBox();

	UPROPERTY(EditAnywhere)
		bool bStopWalking;
	UPROPERTY(EditAnywhere)
		FDialogueUtterance Dialogue;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointGazingLeft;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointGazingRight;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointStandingLeft;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointStandingRight;

	AVivian* Vivian;
	UVHGuide* Guide;

private:

	UFUNCTION()
		void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	FTimerHandle VivianTick;
	FTimerHandle VivianPointing;
	void SetActorTicks();
	void DisablePointing();
};
