// Fill out your copyright notice in the Description page of Project Settings.


#include "Malika.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "VHAnimInstance.h"
#include "Engine.h"
#include "VHGazing.h"
#include "VHGuide.h"
#include "ApproachUser.h"
#include "AtPoI.h"
#include "ToPoI.h"
#include "MeetingPoint.h"
#include "AIController.h"
#include "Components/TextRenderComponent.h"
#include "Helper/CharacterPluginLogging.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "SocialGroups/VHSocialGroups.h"


AMalika::AMalika()
{
	PrimaryActorTick.bCanEverTick = true;
	bTurnToUser = false;
	bDoOnce = true;
	CurrentYaw = 0.0f;
	ZValue = 0;
	SetMalikaDialogues();
}

void AMalika::BeginPlay()
{
	Super::BeginPlay();
	SetActorTicks(false);
	GetGuide();
	UserInSightTimeElapsed = 0;
	UserInSightPhaseDuration = 0;
	bUserWalkedInSight = false;
	if (!Init()) return;
	PreLoadFaceAnimationFiles();
}

// Called every frame
void AMalika::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PrimaryActorTick.TickInterval = 0.3;
	//Walking to points and if group is walking towards a PoI, GazeModel is executed
	if (Guide->State == Guide->ToPoI)
	{
		CheckDistanceToPoint();
		MoveToLocation(*AssignedSlot, MovementComp->MaxWalkSpeed, 1.0f);
		GazeModel(DeltaTime);
		//bDoOnce = true;
	}
	
	if (Guide->State == Guide->ApproachUser)
	{
		//TurnTo(Guide->UserLocation, 60.0f, 5.0f, DeltaTime);
		GazingComp->GazeToUser();
	}

	if (this->GetVelocity().Size() <= 2.0 && (Guide->AtPoI->PoIState == WaitForUser || Guide->AtPoI->PoIState == AtPoi))
	{
		TurnTo(Guide->GetPoi()->GetActorLocation(), 60.0f, 5.0f, 0.02f);
	}
	if (Guide->State == Guide->AtPoI && Guide->AtPoI->PoIState == WaitForUser)
	{
		this->GazingComp->GazeToUser();
	}
	//If group is at PoI or at Meeting Point and guide is speeking
	if ((Guide->State == Guide->AtPoI && Guide->AtPoI->PoIState == AtPoi) || (Guide->State == Guide->MeetingPoint && Guide->SpeechComp->bIsSpeaking))
	{
		GazeModelPoI(DeltaTime);
	}
}


void AMalika::SetActorTicks(bool set)
{
	this->SetActorTickEnabled(set);
}

void AMalika::MoveToLocation(FVector Location, float Speed, float AccepptanceRadius)
{
	MovementComp->MaxWalkSpeed = Speed;
	AIController->MoveToLocation(Location, AccepptanceRadius);
}

bool AMalika::Init()
{
	VH = Cast<AVirtualHuman>(this);
	if (!VH)
	{
		VH_ERROR("[VHGuide::BeginPlay] Couldn't cast to Virtual Human.\n")
			return false;
	}
	VRPawn = Cast<AVirtualRealityPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!VRPawn)
	{
		VH_ERROR("[VHGuide::BeginPlay] The pawn is not a AVirtualRealityPawn.\n")
			return false;
	}
	AIController = Cast<AVirtualHumanAIController>(this->GetController());
	if (!AIController)
	{
		VH_ERROR("[VHGuide]: cannot cast to AIController.\n")
			return false;
	}
	FaceAnimationInstance = this->GetFaceAnimInstance();
	if (!FaceAnimationInstance)
	{
		VH_ERROR("[VHGuide]: Face Animation instance is not valid.\n")
			return false;
	}
	BodyAnimationInstance = this->GetBodyAnimInstance();
	if (!BodyAnimationInstance)
	{
		VH_ERROR("[VHGuide]: Body Animation instance is not valid.\n")
			return false;
	}
	MovementComp = Cast<UCharacterMovementComponent>(this->GetMovementComponent());
	if (!MovementComp)
	{
		VH_ERROR("[VHGuide]: Movement Component is not vaild.\n")
			return false;
	}
	GazingComp = this->FindComponentByClass<UVHGazing>();
	if (!GazingComp)
	{
		VH_ERROR("[VHGuide]: Gazing Component is not vaild.\n")
			return false;
	}
	SpeechComp = this->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp)
	{
		VH_ERROR("[VHGuide]: Dialogue Component is not vaild.\n")
			return false;
	}
	FaceAnimComp = this->FindComponentByClass<UVHFaceAnimation>();
	if (!FaceAnimComp)
	{
		VH_ERROR("[VHGuide]: UVHFaceAnimation Component is not valid.\n")
			return false;
	}
	PointingComp = this->FindComponentByClass<UVHPointing>();
	if (!PointingComp)
	{
		VH_ERROR("[VHGuide]: Pointing Component is not valid\n.")
			return false;
	}
	FacialExpressComp = this->FindComponentByClass<UVHFacialExpressions>();
	if (!FacialExpressComp)
	{
		VH_ERROR("[VHGuide]: UVHFacialExpressions Component is not valid.\n")
			return false;
	}
	return true;
}


void AMalika::SetMalikaDialogues()
{


	FDialogueUtterance MP_1;
	MP_1.Audiofile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP1.wav");
	MP_1.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP1.csv");
	MalikaDialogues.MP1 = MP_1;

	FDialogueUtterance MP_2;
	MP_2.Audiofile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP2.wav");
	MP_2.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP2.csv");
	MalikaDialogues.MP2 = MP_2;

	FDialogueUtterance MP_3;
	MP_3.Audiofile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP3.wav");
	MP_3.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP3.csv");
	MalikaDialogues.MP3 = MP_3;

	FDialogueUtterance MP_4;
	MP_4.Audiofile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP4.wav");
	MP_4.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP4.csv");
	MalikaDialogues.MP4 = MP_4;

	FDialogueUtterance MP_5;
	MP_5.Audiofile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP5.wav");
	MP_5.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/MeetingPoint/Malika_MP5.csv");
	MalikaDialogues.MP5 = MP_5;

	FDialogueUtterance After_You;
	After_You.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_AfterYou.wav");
	After_You.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/AfterYou.csv");
	MalikaDialogues.AfterYou = After_You;

	FDialogueUtterance Go_Ahead;
	Go_Ahead.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_GoAhead.wav");
	Go_Ahead.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/GoAhead.csv");
	MalikaDialogues.GoAhead = Go_Ahead;

	FDialogueUtterance HelloThere;
	HelloThere.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_HelloThere.wav");
	HelloThere.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/HelloThere.csv");
	MalikaDialogues.HelloThere = HelloThere;

	FDialogueUtterance Hey_Come_To_The_Group;
	Hey_Come_To_The_Group.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_HeyComeToTheGroup.wav");
	Hey_Come_To_The_Group.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/HeyWelcome.csv"); //!!!!!!!!!!!!!!!!!!!!!
	MalikaDialogues.HeyComeToTheGroup = Hey_Come_To_The_Group;


	/*
	FDialogueUtterance The_Weather_Is_Perfect;
	The_Weather_Is_Perfect.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_TheWeatherIsPerfect.wav");
	The_Weather_Is_Perfect.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/TheWeatherIsPerfect.csv");
	MalikaDialogues.TheWeatherIsPerfect = The_Weather_Is_Perfect;

	FDialogueUtterance Town_Nearby;
	Town_Nearby.Audiofile = FString("/GroupNavigation/Dialogues/Participants/Malika/Malika_TownNearby.wav");
	Town_Nearby.FaceAnimFile = FString("/GroupNavigation/Dialogues/Participants/SeeTheTown.csv");
	MalikaDialogues.TownNearby = Town_Nearby;
*/
}


void AMalika::PreLoadFaceAnimationFiles()
{
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.MP1.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.MP2.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.MP3.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.MP4.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.MP5.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.AfterYou.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.GoAhead.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.HelloThere.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(MalikaDialogues.HeyComeToTheGroup.FaceAnimFile);
	for (auto DT : DialogueTriggers)
	{
		FaceAnimComp->LoadAnimationFile(DT->Dialogue.FaceAnimFile);
	}
	//FaceAnimComp->LoadAnimationFile(MalikaDialogues.TheWeatherIsPerfect.FaceAnimFile);
	//FaceAnimComp->LoadAnimationFile(MalikaDialogues.TownNearby.FaceAnimFile);
}

void AMalika::GazeModel(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	UserInSightTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;

	//when the user is in the guide's peripheral vision for the first time, gaze at him
	float AgentToUserAngle = FVector::DotProduct(this->GetActorForwardVector(), VRPawn->GetActorLocation() - this->GetActorLocation());
	AgentToUserAngle = FMath::Acos(AgentToUserAngle);
	AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);

	//float AgentToRandomPointAngle;
	FVector RandomVector;

	if (AgentToUserAngle <= PeripheralVisionAngle && !bUserWalkedInSight && UserInSightTimeElapsed >= UserInSightPhaseDuration)
	{
		// when the user is beside or in front of the agent for the first time, look at him
		this->GazingComp->GazeToUser();
		GazeState = EGazeState::Ahead;
		bUserWalkedInSight = true;
	}

	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeState)
		{
		case EGazeState::Ahead:
			FacialExpressComp->SelectedEmotion = Neutral;
			this->GazingComp->StopGazing();
			GazePhaseDuration = FMath::FRandRange(4.0, 6.0);

			// only gaze to the user if he is in visible range
			if (AgentToUserAngle <= PeripheralVisionAngle)
			{
				GazeState = EGazeState::ToUser;
			}
			else
			{
				GazeState = EGazeState::AtRandom;
			}
			break;

		case EGazeState::ToUser:
			this->GazingComp->GazeToUser();
			FacialExpressComp->SelectedEmotion = Happiness;
			GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
			GazeState = EGazeState::AtRandom;
			break;
		case EGazeState::AtRandom:
			FacialExpressComp->SelectedEmotion = Neutral;
			RandomVector.X = FMath::FRandRange(100, 800);
			RandomVector.Y = FMath::FRandRange(300, 800);
			RandomVector.Z = FMath::FRandRange(300, 800);
			//AgentToRandomPointAngle = UKismetMathLibrary::Acos(FVector::DotProduct(this->GetActorForwardVector(), RandomVector) / (this->GetActorForwardVector().Size())*(RandomVector.Size()));
			/*
			AgentToRandomPointAngle = FVector::DotProduct(this->GetActorForwardVector(), RandomVector);
			AgentToRandomPointAngle = FMath::Acos(AgentToRandomPointAngle);
			AgentToRandomPointAngle = FMath::RadiansToDegrees(AgentToRandomPointAngle);
			*/
			//if (AgentToRandomPointAngle <= PeripheralVisionAngle) 
			//{
			this->GazingComp->GazeTo(RandomVector);
			GazePhaseDuration = FMath::FRandRange(2.0, 4.0);
			GazeState = EGazeState::Ahead;
			//}
			//else {

			GazeState = EGazeState::Ahead;
			//}
			break;
		case EGazeState::ToPoI:
			//should not be called
			GazeState = EGazeState::Ahead;
			break;
		default:
			//VH_ERROR("[VHGuide::GazeModel] Unknown Gaze state!\n");
			break;
		}
	}
}

void AMalika::GazeModelPoI(float DeltaTime)
{
	GazeTimeElapsed += DeltaTime;
	UserInSightTimeElapsed += DeltaTime;
	const float PeripheralVisionAngle = 120.0f;
	const FVector Offset = FVector(0, 0, Guide->VH->BaseEyeHeight);

	//when the user is in the guide's peripheral vision for the first time, gaze at him
	float AgentToUserAngle = FVector::DotProduct(this->GetActorForwardVector(), VRPawn->GetActorLocation() - this->GetActorLocation());
	AgentToUserAngle = FMath::Acos(AgentToUserAngle);
	AgentToUserAngle = FMath::RadiansToDegrees(AgentToUserAngle);

	if (GazeTimeElapsed >= GazePhaseDuration)
	{
		GazeTimeElapsed = 0.0f;
		switch (GazeStatePoI)
		{
		case EGazeStatePoI::GazeToGuide:
			FacialExpressComp->SelectedEmotion = Happiness;
			this->GazingComp->GazeToActor(Guide->VH, Offset);
			GazePhaseDuration = FMath::FRandRange(2.0, 4.0);

			// only gaze to the user if he is in visible range
			if (AgentToUserAngle <= PeripheralVisionAngle && Guide->State == Guide->MeetingPoint)
			{
				GazeStatePoI = EGazeStatePoI::GazeToUser;
			}
			else
			{
				GazeStatePoI = EGazeStatePoI::GazeToPoI;
			}
			break;

		case EGazeStatePoI::GazeToUser:
			this->GazingComp->GazeToUser();
			FacialExpressComp->SelectedEmotion = Happiness;
			GazePhaseDuration = FMath::FRandRange(1.0, 2.0);
			if (Guide->State == Guide->AtPoI)
			{
				GazeStatePoI = EGazeStatePoI::GazeToPoI;
			}
			else if (Guide->State == Guide->MeetingPoint)
			{
				GazeStatePoI = EGazeStatePoI::GazeToGuide;
			}
			break;
		case EGazeStatePoI::GazeToPoI:
			FacialExpressComp->SelectedEmotion = Neutral;
			this->GazingComp->GazeTo(Guide->GetPoi()->GetRefPointGazing());
			GazePhaseDuration = FMath::FRandRange(5.0, 7.0);
			GazeStatePoI = EGazeStatePoI::GazeToGuide;
			break;
		default:
			//VH_ERROR("[VHGuide::GazeModel] Unknown Gaze state!\n");
			break;
		}
	}
}

void AMalika::TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime)
{
	// do not turn if moving
	if (this->GetVelocity().Size() > 2)
	{
		return;
	}

	float TurnSpeed = 1.0f;

	// if we want to be more exact, rotation might not converge
	if (EndThreshold < 5)
	{
		EndThreshold = 5;
	}

	// because we need to subtract the world rotation of the agent, we need to convert the values, to be in range from -180 to 180 again
	FRotator BodyRotation = UKismetMathLibrary::MakeRotFromX(VH->GetActorForwardVector());
	const FRotator ToTargetRotation = UKismetMathLibrary::FindLookAtRotation(VH->GetActorLocation(), FVector(Location.X, Location.Y, ZValue)) - BodyRotation;

	float ToTargetYaw = ToTargetRotation.Yaw;
	ToTargetYaw = ToTargetYaw > 180 ? ToTargetYaw - 360.0f : ToTargetYaw;
	ToTargetYaw = ToTargetYaw < -180 ? ToTargetYaw + 360.0f : ToTargetYaw;

	if ((ToTargetYaw > StartThreshold || ToTargetYaw < -StartThreshold) || bTurnToUser)
	{
		bTurnToUser = true;
		if (ToTargetYaw > EndThreshold)
		{
			BodyAnimationInstance->bTurnRight = true;
			BodyAnimationInstance->bTurnLeft = false;
		}
		else if (ToTargetYaw < -EndThreshold)
		{
			BodyAnimationInstance->bTurnRight = false;
			BodyAnimationInstance->bTurnLeft = true;
		}
		else
		{
			BodyAnimationInstance->bTurnRight = false;
			BodyAnimationInstance->bTurnLeft = false;
			ToTargetYaw = CurrentYaw = 0;
			bTurnToUser = false;
		}
		CurrentYaw = FMath::FInterpTo(CurrentYaw, ToTargetYaw, DeltaTime, TurnSpeed);
		this->AddActorWorldRotation(FRotator(0, CurrentYaw, 0));
	}
}

UVHGuide* AMalika::GetGuide()
{
	for (TActorIterator<AVirtualHuman> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
	{
		AVirtualHuman* Character = *CharacterItr;
		VHs.Add(Character);
	}

	for (int i = 0; i < VHs.Num(); i++)
	{
		Guide = Cast<UVHGuide>(VHs[i]->GetComponentByClass(UVHGuide::StaticClass()));
		if (Guide)
		{
			return Guide;
		}
	}
	return Guide;
}


void AMalika::CheckDistanceToPoint()
{
	float Distance = (this->GetActorLocation() - *AssignedSlot).Size();
	if (Distance > 150.0f)
	{
		MovementComp->MaxWalkSpeed = 150;
	}
	else if (Distance > 100.0f) 
	{
		MovementComp->MaxWalkSpeed = 120;
	}
	else {
		MovementComp->MaxWalkSpeed = 100;
	}
}
