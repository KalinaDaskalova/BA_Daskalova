// Fill out your copyright notice in the Description page of Project Settings.

#include "PoI.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "VHGuide.h"
#include "Engine.h"
#include "Helper/CharacterPluginLogging.h"

// Sets default values
APoI::APoI()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	auto Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("RootMesh");
	Mesh->SetupAttachment(Root);
	RefPointGazing = CreateDefaultSubobject<UStaticMeshComponent>("RefPointGazing");
	auto RefAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	RefPointGazing->SetStaticMesh(RefAsset.Object);
	RefPointGazing->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointGazing->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointGazing->SetHiddenInGame(true);

	RefPointStanding = CreateDefaultSubobject<UStaticMeshComponent>("RefPointStanding");
	RefPointStanding->SetStaticMesh(RefAsset.Object);
	RefPointStanding->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointStanding->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointStanding->SetHiddenInGame(true);

	auto Arrow = CreateDefaultSubobject<UArrowComponent>("DirectionArrow");
	Arrow->SetupAttachment(RefPointStanding);
	Arrow->SetWorldScale3D(FVector(2, 2, 2));
	Arrow->SetHiddenInGame(true);

	NoticedMaterial = ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("MaterialInstanceConstant'/Game/Default_Geometry/Materials/MI_Green.MI_Green'")).Object;

}

// Called when the game starts or when spawned
void APoI::BeginPlay()
{
	Super::BeginPlay();

	VRPawn = Cast<AVirtualRealityPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!VRPawn)
	{
		VH_ERROR("[VHGuide::BeginPlay] The pawn is not a AVirtualRealityPawn.\n")
			return;
	}

	if (ChildPoi.Num() > 0)
	{
		bIsParent = true;
		for (int i = 0; i < ChildPoi.Num(); i++)
		{
			APoI* CurrentPoi = ChildPoi[i];
			if (!IsValid(CurrentPoi))
			{
				VH_ERROR("[PointOfInterest]: There is an invalid ChildPoI\n");
				return;
			}
			CurrentPoi->bIsChild = true;
		}
	}

	StandingPositions = InitStandingPositions();
	
}

// Called every frame
void APoI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


TArray<FVector> APoI::InitStandingPositions()
{
	TArray<FVector> StandingPos;
	FVector Center = GetRefPointStanding();
	FVector Forward = RefPointStanding->GetForwardVector();
	// generate 5 points in a half circular arrangement
	for (int i = 0; i < 5; i++)
	{
		FVector ToPoint = Forward.RotateAngleAxis(-70 + (35 * i), FVector(0, 0, 1));
		FVector Point = Center + StandingDistance * ToPoint;
		// fix z value
		Point = FVector(Point.X, Point.Y, Point.Z);
		DrawDebugSphere(GetWorld(), Point, 50.0f, 30.0f, FColor::Green, true);
		StandingPos.Add(Point);
	}
	return StandingPos;
}


FVector APoI::GetRefPointStanding()
{
	return RefPointStanding->GetComponentLocation();
}

FVector APoI::GetRefPointGazing()
{
	// get random point around ref point
	FVector RandDir = UKismetMathLibrary::RandomUnitVector();
	float RandRange = FMath::FRandRange(0, RefPointGazingRadius);

	FVector RandPoint = RefPointGazing->GetComponentLocation() + RandDir * RandRange;

	return RandPoint;
}