// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VHSpeech.h"
#include "Engine/TriggerBox.h"
#include "VHGuide.h"
#include "OmarTriggerBox.generated.h"

/**
 *
 */

class AOmar;

UCLASS()
class STADT_API AOmarTriggerBox : public ATriggerBox
{
	GENERATED_BODY()


public:

	AOmarTriggerBox();

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

	AOmar* Omar;
	UVHGuide* Guide;

private:

	UFUNCTION()
		void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	FTimerHandle OmarTick;
	FTimerHandle OmarPointing;
	void DisablePointing();
	void SetActorTicks();
};
