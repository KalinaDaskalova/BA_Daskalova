// Fill out your copyright notice in the Description page of Project Settings.
#include "QuestionTrigger.h"
#include "VHGuide.h"


AQuestionTrigger::AQuestionTrigger()
{
	FScriptDelegate Del;
	Del.BindUFunction(this, FName("BeginOverlap"));
	this->OnActorBeginOverlap.AddUnique(Del);
	this->SetActorScale3D(FVector(3, 3, 3));
}


void AQuestionTrigger::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	const auto Guide = Cast<UVHGuide>(OtherActor->GetComponentByClass(UVHGuide::StaticClass()));
	//const auto VH = Cast<AVirtualHuman>(OtherActor);
	if (!Guide) return;

	const auto SpeechComp = Guide->VH->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp) return;

	if (bShowQuestions)
	{
		Guide->AllowUserInput();
	}
	else if (!bShowQuestions)
	{
		Guide->DisableUserInput();
	}
	
}

