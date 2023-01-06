// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AWaypoint.generated.h"

UCLASS()
class STADT_API AAWaypoint : public AActor
{
	GENERATED_BODY()

public:
	int GetWaypointOrder();

	
private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
		int WaypointOrder;

};
