// Fill out your copyright notice in the Description page of Project Settings.


#include "VHParticipant.h"
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



AVHParticipant::AVHParticipant()
{
	bTurnToUser = false;
	CurrentYaw = 0.0f;
	ZValue = 0;
}


void AVHParticipant::BeginPlay()
{
	Super::BeginPlay();
	//SetActorTicks(false);
	GetGuide();
	UserInSightTimeElapsed = 0;
	UserInSightPhaseDuration = 0;
	bUserWalkedInSight = false;
	//Participants = GetParticipants();
	if (!Init()) return;
}

// Called every frame
void AVHParticipant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//If group is walking towards a PoI, GazeModel is executed
	if (Guide->State == Guide->ToPoI)
	{
		GazeModel(DeltaTime);
	}
	if (Guide->State == Guide->ApproachUser)
	{
		//TurnTo(Guide->UserLocation, 60.0f, 5.0f, DeltaTime);
		GazingComp->GazeToUser();
	}
	if (this->GetVelocity().Size() <= 2.0 && Guide->State == Guide->AtPoI)
	{
		TurnTo(Guide->CurrentPoI->GetActorLocation(), 60.0f, 5.0f, DeltaTime);
	}
	/*
	if (Guide->State == Guide->AtPoI && Guide->AtPoI->PoIState == WaitForUser)
	{
		this->GazingComp->GazeToUser();
	}
	*/
	//If group is at PoI or at Meeting Point and guide is speeking
	if ((Guide->State == Guide->AtPoI /*&& Guide->AtPoI->PoIState == AtPoi)*/ || Guide->State == Guide->MeetingPoint) && Guide->SpeechComp->bIsSpeaking)
	{
		GazeModelPoI(DeltaTime);
	}
}



void AVHParticipant::SetActorTicks(bool set)
{
	this->SetActorTickEnabled(set);
}

void AVHParticipant::MoveToLocation(FVector Location, float Speed, float AccepptanceRadius)
{
	MovementComp->MaxWalkSpeed = Speed;
	AIController->MoveToLocation(Location, AccepptanceRadius);
}


bool AVHParticipant::Init()
{
	/*
	VH = Cast<AVirtualHuman>(GetOwner());
	if (!VH)
	{
		VH_ERROR("[VHGuide]: Cannot cast to AVirtualHuamn.\n")
			return false;
	}
	VHPawn = Cast<APawn>(GetOwner());
	if (!VHPawn)
	{
		VH_ERROR("[VHGuide]: Cannot cast to APawn.\n")
			return false;
	}
	*/
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
	FacialExpressComp = this->FindComponentByClass<UVHFacialExpressions>();
	if (!FacialExpressComp)
	{
		VH_ERROR("[VHGuide]: UVHFacialExpressions Component is not valid.\n")
			return false;
	}
	return true;
}



void AVHParticipant::GazeModel(float DeltaTime)
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


void AVHParticipant::GazeModelPoI(float DeltaTime)
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
			this->GazingComp->GazeToActor(Guide->VH, Offset);
			GazePhaseDuration = FMath::FRandRange(2.0, 4.0);

			// only gaze to the user if he is in visible range
			if (AgentToUserAngle <= PeripheralVisionAngle)
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
			GazePhaseDuration = FMath::FRandRange(1.0, 2.0);
			GazeStatePoI = EGazeStatePoI::GazeToPoI;
			break;
		if(Guide->State == Guide->AtPoI)
		case EGazeStatePoI::GazeToPoI:
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




void AVHParticipant::TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime)
{
	// do not turn if moving
	if (this->GetVelocity().Size() > 2)
	{
		return;
	}

	float TurnSpeed = 0.05f;

	// if we want to be more exact, rotation might not converge
	if (EndThreshold < 5)
	{
		EndThreshold = 5;
	}

	// because we need to subtract the world rotation of the agent, we need to convert the values, to be in range from -180 to 180 again
	FRotator BodyRotation = UKismetMathLibrary::MakeRotFromX(this->GetActorForwardVector());
	const FRotator ToTargetRotation = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), FVector(Location.X, Location.Y, ZValue)) - BodyRotation;

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

UVHGuide* AVHParticipant::GetGuide()
{
	for (TActorIterator<AVirtualHuman> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
	{
		AVirtualHuman* Character = *CharacterItr;
		VHs.Add(Character);
	}
	
	for (int i = 0; i < VHs.Num(); i++)
	{
		Guide = Cast<UVHGuide>(VHs[i]->GetComponentByClass(UVHGuide::StaticClass()));
		if(Guide)
		{
			return Guide;
		}
	}
	return Guide;
}