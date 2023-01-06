// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "VHSpeech.h"
#include "GuideTriggerBox.generated.h"

/**
 *
 */


UCLASS()
class STADT_API AGuideTriggerBox : public ATriggerBox
{
	GENERATED_BODY()


public:

	AGuideTriggerBox();

	UPROPERTY(EditAnywhere)
		FDialogueUtterance Dialogue;

private:

	UFUNCTION()
		void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

};
