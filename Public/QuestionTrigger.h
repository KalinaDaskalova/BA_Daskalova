// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "QuestionTrigger.generated.h"

/**
 *
 */
UCLASS()
class STADT_API AQuestionTrigger : public ATriggerBox
{
	GENERATED_BODY()


public:

	AQuestionTrigger();

	UPROPERTY(EditAnywhere)
		bool bShowQuestions;

private:

	UFUNCTION()
		void BeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

};
