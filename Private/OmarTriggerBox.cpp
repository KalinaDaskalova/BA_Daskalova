// Fill out your copyright notice in the Description page of Project Settings.
#include "OmarTriggerBox.h"
#include "Engine.h"
#include "ToPoI.h"
#include "Omar.h"


AOmarTriggerBox::AOmarTriggerBox()
{
	Omar;
	FScriptDelegate Del;
	Del.BindUFunction(this, FName("BeginOverlap"));
	this->OnActorBeginOverlap.AddUnique(Del);
	this->SetActorScale3D(FVector(3, 3, 3));
	auto Root = CreateDefaultSubobject<USceneComponent>("Root");
	this->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	SetRootComponent(Root);
	auto RefAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	RefPointGazingLeft = CreateDefaultSubobject<UStaticMeshComponent>("RefPointGazingLeft");
	RefPointGazingLeft->SetStaticMesh(RefAsset.Object);
	RefPointGazingLeft->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointGazingLeft->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointGazingLeft->SetHiddenInGame(true);
	RefPointGazingRight = CreateDefaultSubobject<UStaticMeshComponent>("RefPointGazingRight");
	RefPointGazingRight->SetStaticMesh(RefAsset.Object);
	RefPointGazingRight->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointGazingRight->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointGazingRight->SetHiddenInGame(true);
	RefPointStandingLeft = CreateDefaultSubobject<UStaticMeshComponent>("RefPointStandingLeft");
	RefPointStandingLeft->SetStaticMesh(RefAsset.Object);
	RefPointStandingLeft->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointStandingLeft->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointStandingLeft->SetHiddenInGame(true);
	RefPointStandingRight = CreateDefaultSubobject<UStaticMeshComponent>("RefPointStandingRight");
	RefPointStandingRight->SetStaticMesh(RefAsset.Object);
	RefPointStandingRight->SetWorldScale3D(FVector(0.4, 0.4, 0.4));
	RefPointStandingRight->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	RefPointStandingRight->SetHiddenInGame(true);
}


void AOmarTriggerBox::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Dialogue.Audiofile == "" || Dialogue.FaceAnimFile == "") return;
	Omar = Cast<AOmar>(OtherActor);
	if (!Omar) return;
	Guide = Omar->GetGuide();

	const auto SpeechComp = Omar->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp) return;

	SpeechComp->PlayDialogue(Dialogue, 0.1, false);

	if (bStopWalking)
	{
		if (Omar->AssignedSlot == Guide->ToPoI->PositionsPointer[0] || Omar->AssignedSlot == Guide->ToPoI->PositionsPointer[2])
		{
			Omar->SetActorTickEnabled(false);
			Omar->MoveToLocation(RefPointStandingLeft->GetComponentLocation(), 70, 10.0f);
			Omar->GazingComp->GazeTo(RefPointGazingLeft->GetComponentLocation());
			Omar->PointingComp->PointingTarget = RefPointGazingLeft->GetComponentLocation();
			Omar->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(OmarPointing, this, &AOmarTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(OmarTick, this, &AOmarTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
		else
		{
			Omar->SetActorTickEnabled(false);
			Omar->MoveToLocation(RefPointStandingRight->GetComponentLocation(), 70, 10.0f);
			Omar->GazingComp->GazeTo(RefPointGazingRight->GetComponentLocation());
			Omar->PointingComp->PointingTarget = RefPointGazingRight->GetComponentLocation();
			Omar->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(OmarPointing, this, &AOmarTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(OmarTick, this, &AOmarTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
	}
}

void AOmarTriggerBox::SetActorTicks()
{
	Omar->SetActorTickEnabled(true);
}


void AOmarTriggerBox::DisablePointing()
{
	Omar->PointingComp->bIsPointing = false;
}

