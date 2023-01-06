// Fill out your copyright notice in the Description page of Project Settings.
#include "VivianTriggerBox.h"
#include "Engine.h"
#include "ToPoI.h"
#include "Vivian.h"


AVivianTriggerBox::AVivianTriggerBox()
{
	Vivian;
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


void AVivianTriggerBox::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Dialogue.Audiofile == "" || Dialogue.FaceAnimFile == "") return;
	Vivian = Cast<AVivian>(OtherActor);
	if (!Vivian) return;
	Guide = Vivian->GetGuide();

	const auto SpeechComp = Vivian->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp) return;

	SpeechComp->PlayDialogue(Dialogue, 0.1, false);
	if (bStopWalking)
	{
		if (Vivian->AssignedSlot == Guide->ToPoI->PositionsPointer[0] || Vivian->AssignedSlot == Guide->ToPoI->PositionsPointer[1])
		{
			Vivian->SetActorTickEnabled(false);
			Vivian->MoveToLocation(RefPointStandingLeft->GetComponentLocation(), 70, 10.0f);
			Vivian->GazingComp->GazeTo(RefPointGazingLeft->GetComponentLocation());
			Vivian->PointingComp->PointingTarget = RefPointGazingLeft->GetComponentLocation();
			Vivian->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(VivianPointing, this, &AVivianTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(VivianTick, this, &AVivianTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
		else
		{
			Vivian->SetActorTickEnabled(false);
			Vivian->MoveToLocation(RefPointStandingRight->GetComponentLocation(), 70, 10.0f);
			Vivian->GazingComp->GazeTo(RefPointGazingRight->GetComponentLocation());
			Vivian->PointingComp->PointingTarget = RefPointGazingRight->GetComponentLocation();
			Vivian->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(VivianPointing, this, &AVivianTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(VivianTick, this, &AVivianTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
	}
}

void AVivianTriggerBox::SetActorTicks()
{
	Vivian->SetActorTickEnabled(true);
}


void AVivianTriggerBox::DisablePointing()
{
	Vivian->PointingComp->bIsPointing = false;
}



