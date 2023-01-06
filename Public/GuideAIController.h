// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "VirtualHuman.h"
#include "GuideAIController.generated.h"

/**
 * 
 */
class UVHGuide;

UCLASS()
class STADT_API AGuideAIController : public AAIController
{
	GENERATED_BODY()

public:
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void BeginPlay() override;

	UVHGuide* GetGuide();

	UVHGuide* VHGuide;

	UVHGuide* Guide;

	TArray<AVirtualHuman*> VHs;
};
