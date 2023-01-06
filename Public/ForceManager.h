// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VirtualHuman.h"
#include "VHGuide.h"
#include "Components/ActorComponent.h"
#include "ForceManager.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STADT_API UForceManager : public UActorComponent
{
	GENERATED_BODY()

public:

	UForceManager();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<AVirtualHuman*> GetVHs();
	TArray<AVirtualHuman*> VHs;
	TArray<AVirtualHuman*> VirtualHumans;
	TArray<FVector> PositionsOnPoI;

	UPROPERTY()
		AVirtualHuman* VH;
	float VHToUserAngle;
	FVector GetWalkingForce();
	FVector GetPositionForce();
	FVector GetPoIForce();
	FVector GetSideForce();
	FVector GetVHSideForce(AVirtualHuman* AVirtualHuman);
	FVector GetBackwardsForce(AVirtualHuman* AVirtualHuman);

	UVHGuide* Guide;
	UVHGuide* GetGuide();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
