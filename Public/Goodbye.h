// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "States.h"
#include "VHGuide.h"
#include "Vivian.h"
#include "Malika.h"
#include "Omar.h"
#include "Goodbye.generated.h"

/**
 * 
 */
UCLASS()
class STADT_API UGoodbye : public UObject, public IStates
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	UGoodbye();
	virtual void Tick(float DeltaTime) override;
	virtual void OnStateChange() override;
	//virtual void OnInterruptionBegin() override;
	virtual void GazeModel(float DeltaTime) override;

	UVHGuide* Guide;
	TArray <AVirtualHuman*> Followers;
};
