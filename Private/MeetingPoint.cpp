// Fill out your copyright notice in the Description page of Project Settings.



#include "MeetingPoint.h"
#include "SocialGroups/SocialGroup.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Engine.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "VHGazing.h"
#include "VHSpeech.h"
#include "ToPoI.h"
#include "Engine/AssetManager.h"
#include "Helper/CharacterPluginLogging.h"
#include "SoundSource/VAAudiofileSourceComponent.h"


// Sets default values
UMeetingPoint::UMeetingPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	bdoOnce = true;
	bdoOnceTwo = true;
	bdoOnceThree = true;
	bDoOnceFour = true;
	bGuideIsSpeaking = false;
	Guide = Cast<UVHGuide>(GetOuter());
	if (IsValid(Guide)) Guide->PrintDebug("State UAtPoi created");
	//MP = Cast<AMP>(UGameplayStatics::GetActorOfClass(GetWorld(), AMP::StaticClass()));
	MPState = Conversation;

}


// Called when the game starts or when spawned
void UMeetingPoint::OnStateChange()
{
	SG = Cast<ASocialGroup>(UGameplayStatics::GetActorOfClass(GetWorld(), ASocialGroup::StaticClass()));
	Malika = Cast<AMalika>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Omar = Cast<AOmar>(UGameplayStatics::GetActorOfClass(GetWorld(), AOmar::StaticClass()));
	Vivian = Cast<AVivian>(UGameplayStatics::GetActorOfClass(GetWorld(), AVivian::StaticClass()));
}



// Called every frame
void UMeetingPoint::Tick(float DeltaTime)
{

	//Chech if user is here
	//if user in group->Play introduction
	switch (MPState)
	{
	case(Conversation):
		//Timer to play conversation montage in two random participants of the social group
		//repeat the timer until user is in group
		if (bdoOnceTwo)
		{
			//PlayConversationDelegate = FTimerDelegate::CreateUObject(this, &UMeetingPoint::PlayConversation, Guide->Montages[1]);
			GetWorld()->GetTimerManager().SetTimer(PlayConversationTimer, this, &UMeetingPoint::PlayConversation, 3.0f, false, 0.5f);
			bdoOnceTwo = false;
		}
		//Check if user is looking at group
		if (UserOrientedToGroup())
		{
			if (bdoOnce) {
				//Delay playing of waving animation, because otherwise it plays before the guide has turned to user
				Guide->GazingComp->GazeToUser();
				PlayMontageDelegate = FTimerDelegate::CreateUObject(this, &UMeetingPoint::PlayWaving, Guide->Montages[0]);
				GetWorld()->GetTimerManager().SetTimer(PlayWavingTimer, PlayMontageDelegate, 0.5f, false, 1.0f);
				bdoOnce = false;
			}
		}

		if (SG->bPlayerIsInGroup)
		{
			Guide->PrintDebug("[VHGuide]:User joined the group.");
			MPState = UserInGroup;
		}
		break;
	case(UserInGroup):
		//Stop timer that plays conversation animation among participants
		Vivian->GazingComp->GazeToUser();
		Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.HeyWelcome, 1.0f, true);
		if (!Vivian->SpeechComp->bIsSpeaking)
		{
			if (!Malika->SpeechComp->bIsSpeaking)
			{
				if (!Omar->SpeechComp->bIsSpeaking)
				{
					if (bdoOnceThree)
					{
						GetWorld()->GetTimerManager().ClearTimer(PlayVivianDialogue1);
						GetWorld()->GetTimerManager().ClearTimer(PlayVivianDialogue2);
						GetWorld()->GetTimerManager().ClearTimer(PlayVivianDialogue3);
						GetWorld()->GetTimerManager().ClearTimer(PlayMalikaDialogue1);
						GetWorld()->GetTimerManager().ClearTimer(PlayMalikaDialogue2);
						GetWorld()->GetTimerManager().ClearTimer(PlayMalikaDialogue3);
						GetWorld()->GetTimerManager().ClearTimer(PlayMalikaDialogue4);
						GetWorld()->GetTimerManager().ClearTimer(PlayMalikaDialogue5);
						GetWorld()->GetTimerManager().ClearTimer(PlayOmarDialogue1);
						GetWorld()->GetTimerManager().ClearTimer(PlayOmarDialogue2);
						GetWorld()->GetTimerManager().ClearTimer(PlayOmarDialogue3);
						GetWorld()->GetTimerManager().ClearTimer(PlayOmarDialogue4);
						GetWorld()->GetTimerManager().ClearTimer(PlayOmarDialogue5);
						Vivian->SpeechComp->StopDialogue();
						Malika->SpeechComp->StopDialogue();
						Omar->SpeechComp->StopDialogue();
						//Guide->Followers[1]->GazingComp->GazeToUser();
						Vivian->GetBodyAnimInstance()->Montage_Play(Guide->Montages[0]);
						Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.Greeting, 3.0f, true);
						MPState = GiveInformation;
						bdoOnceThree = false;

					}
				}
			}
		}
		break;
	case (GiveInformation):
		if ((Guide->SpeechComp->bIsSpeaking))
		{
			bGuideIsSpeaking = true;
		}
		if (bGuideIsSpeaking)
		{
			if (!(Guide->SpeechComp->bIsSpeaking))
			{
				SG->SetActorTickEnabled(false);
				SG->SetVHSocialGroupsTicks(false);
				Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.OkayLetsGo, 0.1f, true);
				Guide->ChangeState(Guide->ToPoI);
			}
		}
		break;
	}

}

//Set timers to activate dialogue
void UMeetingPoint::PlayConversation()
{
	//int i = FMath::RandRange(0, 2);
	GetWorld()->GetTimerManager().SetTimer(PlayVivianDialogue1, this, &UMeetingPoint::VivianDialogue1, 0.3f, false, 4.f);
	GetWorld()->GetTimerManager().SetTimer(PlayVivianDialogue2, this, &UMeetingPoint::VivianDialogue2, 0.3f, false, 8.f);
	GetWorld()->GetTimerManager().SetTimer(PlayVivianDialogue3, this, &UMeetingPoint::VivianDialogue3, 0.3f, false, 11.f);

	GetWorld()->GetTimerManager().SetTimer(PlayMalikaDialogue1, this, &UMeetingPoint::MalikaDialogue1, 3.0f, false, 14.f);
	GetWorld()->GetTimerManager().SetTimer(PlayMalikaDialogue2, this, &UMeetingPoint::MalikaDialogue2, 3.0f, false, 17.f);
	GetWorld()->GetTimerManager().SetTimer(PlayMalikaDialogue3, this, &UMeetingPoint::MalikaDialogue3, 3.0f, false, 20.f);
	GetWorld()->GetTimerManager().SetTimer(PlayMalikaDialogue4, this, &UMeetingPoint::MalikaDialogue4, 3.0f, false, 23.f);
	GetWorld()->GetTimerManager().SetTimer(PlayMalikaDialogue5, this, &UMeetingPoint::MalikaDialogue5, 3.0f, false, 26.f);

	GetWorld()->GetTimerManager().SetTimer(PlayOmarDialogue1,this, &UMeetingPoint::OmarDialogue1, 3.0f, false, 30.f);
	GetWorld()->GetTimerManager().SetTimer(PlayOmarDialogue2, this, &UMeetingPoint::OmarDialogue2, 3.0f, false, 32.f);
	GetWorld()->GetTimerManager().SetTimer(PlayOmarDialogue3, this, &UMeetingPoint::OmarDialogue3, 3.0f, false, 36.f);
	GetWorld()->GetTimerManager().SetTimer(PlayOmarDialogue4, this, &UMeetingPoint::OmarDialogue4, 3.0f, false, 38.f);
	GetWorld()->GetTimerManager().SetTimer(PlayOmarDialogue5, this, &UMeetingPoint::OmarDialogue5, 3.0f, false, 42.f);
	//Guide->Followers[i]->GetBodyAnimInstance()->Montage_Play(ConversationMontage);
}


void UMeetingPoint::VivianDialogue1()
{
	Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.MP1, 0.1f, true);
}

void UMeetingPoint::VivianDialogue2()
{
	Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.MP2, 0.1f, true);
}

void UMeetingPoint::VivianDialogue3()
{
	Vivian->SpeechComp->PlayDialogue(Vivian->VivianDialogues.MP3, 0.1f, true);
}

void UMeetingPoint::MalikaDialogue1()
{
	Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.MP1, 0.1f, true);
}

void UMeetingPoint::MalikaDialogue2()
{
	Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.MP2, 0.1f, true);
}

void UMeetingPoint::MalikaDialogue3()
{
	Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.MP3, 0.1f, true);
}

void UMeetingPoint::MalikaDialogue4()
{
	Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.MP4, 0.1f, true);
}

void UMeetingPoint::MalikaDialogue5()
{
	Malika->SpeechComp->PlayDialogue(Malika->MalikaDialogues.MP5, 0.1f, true);
}

void UMeetingPoint::OmarDialogue1()
{
	Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.MP1, 0.1f, true);
}

void UMeetingPoint::OmarDialogue2()
{
	Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.MP2, 0.1f, true);
}

void UMeetingPoint::OmarDialogue3()
{
	Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.MP3, 0.1f, true);
}

void UMeetingPoint::OmarDialogue4()
{
	Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.MP4, 0.1f, true);
}

void UMeetingPoint::OmarDialogue5()
{
	Omar->SpeechComp->PlayDialogue(Omar->OmarDialogues.MP5, 0.1f, true);
}


void UMeetingPoint::PlayWaving(UAnimMontage* WavingMontage)
{
	Guide->SpeechComp->PlayDialogue(Guide->GuideDialogues.Noticed, 0.1f, false);
	GazeState = ToUser;
	Guide->VH->GetBodyAnimInstance()->Montage_Play(WavingMontage);
}

bool UMeetingPoint::UserOrientedToGroup()
{
	FVector UserForward = Guide->VRPawn->Head->GetForwardVector();
	FVector UserToGroupCenter = Guide->UserLocation - Guide->VH->GetActorLocation();
	FVector2D UserForwardXY = FVector2D(UserForward.X, UserForward.Y);
	FVector2D UserToGroupCenterXY = FVector2D(UserToGroupCenter.X, UserToGroupCenter.Y);

	UserForwardXY.Normalize();
	UserToGroupCenterXY.Normalize();

	float Angle = FMath::Abs(FVector2D::DotProduct(UserForwardXY, UserToGroupCenterXY));
	Angle = FMath::Acos(Angle);
	Angle = FMath::RadiansToDegrees(Angle);
	return Angle < 30.0f;
}



void UMeetingPoint::GazeModel(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;
	int i;
	float AgentToUserAngle;

	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeState)
		{
		case EGazeState::ToUser:
			Guide->GazingComp->GazeToUser();
			GazePhaseDuration = FMath::FRandRange(2.0, 6.0);
			GazeState = EGazeState::ToParticipant;
			break;
		case EGazeState::ToPoI:
			//should not be called
			GazeState = EGazeState::ToParticipant;
			break;
		case EGazeState::ToParticipant:
			i = FMath::RandRange(0, 2);
			AgentToUserAngle = FVector::DotProduct(Guide->VH->GetActorForwardVector(), Guide->Followers[i]->GetActorLocation() - Guide->VH->GetActorLocation());
			AgentToUserAngle = FMath::Acos(AgentToUserAngle);
			AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);
			if (AgentToUserAngle <= PeripheralVisionAngle) {
				Guide->GazingComp->GazeToActor(Guide->Followers[i], FVector(0, 0, 120.0f));
				GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
				GazeState = EGazeState::ToUser;
			}
			else {
				GazeState = EGazeState::ToUser;
			}
			break;
		case EGazeState::Ahead:
			// should not be called
			GazeState = EGazeState::ToUser;
			break;
		case EGazeState::AtRandom:
			//should not be called
			GazeState = EGazeState::Ahead;
			break;
		default:
			//VH_ERROR("[VHGuide::GazeModel] Unknown Gaze state!\n");
			break;
		}
	}
}

