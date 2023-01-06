// Fill out your copyright notice in the Description page of Project Settings.
#include "MalikaTriggerBox.h"
#include "Engine.h"
#include "ToPoI.h"
#include "Malika.h"


AMalikaTriggerBox::AMalikaTriggerBox()
{
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


void AMalikaTriggerBox::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Dialogue.Audiofile == "" || Dialogue.FaceAnimFile == "") return;
	Malika = Cast<AMalika>(OtherActor);
	if (!Malika) return;
	Guide = Malika->GetGuide();

	const auto SpeechComp = Malika->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp) return;

	SpeechComp->PlayDialogue(Dialogue, 0.1, false);

	if (bStopWalking)
	{
		if (Malika->AssignedSlot == Guide->ToPoI->PositionsPointer[0] || Malika->AssignedSlot == Guide->ToPoI->PositionsPointer[2])
		{
			Malika->SetActorTickEnabled(false);
			Malika->MoveToLocation(RefPointStandingLeft->GetComponentLocation(), 70, 10.0f);
			Malika->GazingComp->GazeTo(RefPointGazingLeft->GetComponentLocation());
			Malika->PointingComp->PointingTarget = RefPointGazingLeft->GetComponentLocation();
			Malika->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(MalikaPointing, this, &AMalikaTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(MalikaTick, this, &AMalikaTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
		else {
			Malika->SetActorTickEnabled(false);
			Malika->MoveToLocation(RefPointStandingRight->GetComponentLocation(), 70, 10.0f);
			Malika->GazingComp->GazeTo(RefPointGazingRight->GetComponentLocation());
			Malika->PointingComp->PointingTarget = RefPointGazingRight->GetComponentLocation();
			Malika->PointingComp->bIsPointing = true;
			GetWorld()->GetTimerManager().SetTimer(MalikaPointing, this, &AMalikaTriggerBox::DisablePointing, 1.0f, false, 1.0f);
			GetWorld()->GetTimerManager().SetTimer(MalikaTick, this, &AMalikaTriggerBox::SetActorTicks, 5.0f, false, 5.0f);
		}
	}
}

void AMalikaTriggerBox::SetActorTicks()
{
	Malika = Cast<AMalika>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Malika->SetActorTickEnabled(true);
}


void AMalikaTriggerBox::DisablePointing()
{
	Malika = Cast<AMalika>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Malika->PointingComp->bIsPointing = false;
}
