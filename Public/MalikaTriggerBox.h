// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VHSpeech.h"
#include "Engine/TriggerBox.h"
#include "VHGuide.h"
#include "MalikaTriggerBox.generated.h"

/**
 *
 */

class AMalika;

UCLASS()
class STADT_API AMalikaTriggerBox : public ATriggerBox
{
	GENERATED_BODY()


public:

	AMalikaTriggerBox();

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
	AMalika* Malika;
	UVHGuide* Guide;

private:

	UFUNCTION()
		void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	FTimerHandle MalikaTick;
	FTimerHandle MalikaPointing;
	void DisablePointing();
	void SetActorTicks();
};
