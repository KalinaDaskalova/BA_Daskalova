// Fill out your copyright notice in the Description page of Project Settings.
#include "GuideTriggerBox.h"
#include "VHSpeech.h"
#include "VHGuide.h"


AGuideTriggerBox::AGuideTriggerBox()
{
	FScriptDelegate Del;
	Del.BindUFunction(this, FName("BeginOverlap"));
	this->OnActorBeginOverlap.AddUnique(Del);
	this->SetActorScale3D(FVector(3, 3, 3));
}


void AGuideTriggerBox::BeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (Dialogue.Audiofile == "" || Dialogue.FaceAnimFile == "") return;
	const auto Guide = Cast<UVHGuide>(OtherActor->GetComponentByClass(UVHGuide::StaticClass()));
	//const auto VH = Cast<AVirtualHuman>(OtherActor);
	if (!Guide) return;

	const auto SpeechComp = Guide->VH->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp) return;

	if (!Guide->SpeechComp->bIsSpeaking)
	{
		SpeechComp->PlayDialogue(Dialogue, 0.1, false);
	}

}

