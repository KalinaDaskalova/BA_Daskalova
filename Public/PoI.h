// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VHSpeech.h"
#include "GuideTriggerBox.h"
#include "Pawn/VirtualRealityPawn.h"
#include "PoI.generated.h"



USTRUCT(BlueprintType)
struct FPoiDialogue
{
	GENERATED_BODY()

	// the approach monologue will be given by the guide, at the end of the previous point of interest (poi)
	// or before the guide arrives at this poi
	// e.g.: "we will know checkout, the city hall"
	UPROPERTY(EditAnywhere)
		FDialogueUtterance ApproachMon;

	//Monologue that the guide is going to give the participants
	UPROPERTY(EditAnywhere)
		FDialogueUtterance Monologue;

};


UCLASS()
class STADT_API APoI : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APoI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// contains the information, the guide will provide to the user
	UPROPERTY(EditAnywhere)
		FPoiDialogue Dialogue;
	UPROPERTY()
		TArray<FVector> Positions;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointGazing;
	UPROPERTY(EditAnywhere)
		float RefPointGazingRadius = 10;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RefPointStanding;
	UPROPERTY(EditAnywhere)
		float StandingDistance = 220.0f;
	UPROPERTY(EditAnywhere)
		TArray<APoI*> ChildPoi;
	UPROPERTY()
		UMaterialInstance* NoticedMaterial;
	UPROPERTY(EditAnywhere)
		TArray<AGuideTriggerBox*> ResponseTriggers;

	AVirtualRealityPawn* VRPawn;
	UPROPERTY()
		TArray<FVector> StandingPositions;
	TArray<FVector> InitStandingPositions();
	UFUNCTION(BlueprintCallable)
		FVector GetRefPointGazing();
	UFUNCTION(BlueprintCallable)
		FVector GetRefPointStanding();

	bool bIsChild;
	bool bIsParent;

	


};
