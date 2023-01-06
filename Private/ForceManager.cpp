// Fill out your copyright notice in the Description page of Project Settings.

#include "ForceManager.h"
#include "Engine.h"
#include "ToPoI.h"
#include "AtPoI.h"


UForceManager::UForceManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	if (IsValid(Guide)) Guide->PrintDebug("ForceManager created");
}

void UForceManager::BeginPlay()
{
	Super::BeginPlay();
	VHs = GetVHs();
	Guide = GetGuide();
	VH = Cast<AVirtualHuman>(GetOwner());
}

void UForceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	FVector Movement;
	
	//If we are at PoI and the user is still not there
	if (Guide->State == Guide->AtPoI && (Guide->AtPoI->PoIState == AtPoi || Guide->AtPoI->PoIState == WaitForUser)) //&& !Guide->AtPoI->CheckUserLocation())
	{
		Movement = GetPositionForce() + GetPoIForce();
			if (Movement.Size() > 2.0f)
			{
				Movement.Normalize();
				VH->AddMovementInput(Movement, 0.3f);
			}
	}


	for (int i = 0; i < VHs.Num(); i++)
	{
		VHToUserAngle = FVector::DotProduct(VHs[i]->GetActorForwardVector(), Guide->VRPawn->GetActorLocation() - VHs[i]->GetActorLocation());
		VHToUserAngle = FMath::Acos(VHToUserAngle);
		VHToUserAngle = FMath::RadiansToDegrees(VHToUserAngle);
		if (VHToUserAngle < 40 && Guide->State == Guide->ToPoI)
		{
			Movement = GetVHSideForce(VHs[i]) + GetBackwardsForce(VHs[i]);
			if (Movement.Size() > 2.0f)
			{
				Movement.Normalize();
				VHs[i]->AddMovementInput(Movement, 0.7f);
			}
		}
		else if (VHToUserAngle >= 40 && (Guide->State == Guide->ToPoI || Guide->AtPoI->PoIState == ApproachingPoI))
		{
			Movement = GetWalkingForce();
			if (Movement.Size() > 2.0f)
			{
				Movement.Normalize();
				VH->AddMovementInput(Movement, 0.3f);
			}
		}
	}

	
	
}

FVector UForceManager::GetVHSideForce(AVirtualHuman* AVirtualHuman)
{
	FVector Force = FVector::ZeroVector;
	FVector CurrentAgent = AVirtualHuman->GetActorLocation();
	FVector User = Guide->VRPawn->Head->GetComponentLocation();
	FVector Distance = User - CurrentAgent;

	if (Distance.Size() < 170.0f && Distance.Size() > 0)
	{
		float ToDesiredDistance = 170.0f - Distance.Size();
		Distance.Normalize();
		Force += Distance * ToDesiredDistance;
	}

	return Force.RotateAngleAxis(90, FVector(0, 0, 1));
}


FVector UForceManager::GetBackwardsForce(AVirtualHuman* AVirtualHuman)
{
	FVector Force = FVector::ZeroVector;
	FVector CurrentAgent = AVirtualHuman->GetActorLocation();
	FVector User = Guide->VRPawn->Head->GetComponentLocation();
	FVector Distance = User - CurrentAgent;

	if (Distance.Size() < 170.0f && Distance.Size() > 0)
	{
		float ToDesiredDistance = 170.0f - Distance.Size();
		Distance.Normalize();
		Force += Distance * ToDesiredDistance;
	}

	return -Force;
}


FVector UForceManager::GetPositionForce()
{
	
	FVector Force = FVector::ZeroVector;

	for (int i = 0; i < VHs.Num(); i++)
	{
		FVector CurrentAgent = VH->GetActorLocation();
		FVector OtherAgent = VHs[i]->GetActorLocation();
		FVector Distance = OtherAgent - CurrentAgent;

		if (Distance.Size() < 120.0f /*ConversationDistance*/ && Distance.Size() > 0)
		{
			float ToDesiredDistance = 120.0f - Distance.Size();
			Distance.Normalize();
			Force += Distance * ToDesiredDistance;
		}

		CurrentAgent = VH->GetActorLocation();
		OtherAgent = Guide->VRPawn->Head->GetComponentLocation();
		Distance = OtherAgent - CurrentAgent;

		if (Distance.Size() < 120.0f && Distance.Size() > 0)
		{
			float ToDesiredDistance = 120.0f - Distance.Size();
			Distance.Normalize();
			Force += Distance * ToDesiredDistance;
		}

	}

	return -Force;
}


FVector UForceManager::GetWalkingForce()
{
	FVector Force = FVector::ZeroVector;

	for (int i = 0; i < VHs.Num(); i++)
	{
		FVector CurrentAgent = VH->GetActorLocation();
		FVector OtherAgent = VHs[i]->GetActorLocation();
		FVector Distance = OtherAgent - CurrentAgent;

		if (Distance.Size() < 130.0f /*ConversationDistance*/ && Distance.Size() > 0)
		{
			float ToDesiredDistance = 130.0f - Distance.Size();
			Distance.Normalize();
			Force += Distance * ToDesiredDistance;
		}

		CurrentAgent = VH->GetActorLocation();
		OtherAgent = Guide->VRPawn->Head->GetComponentLocation();
		Distance = OtherAgent - CurrentAgent;

		if (Distance.Size() < 130.0f && Distance.Size() > 0)
		{
			float ToDesiredDistance = 130.0f - Distance.Size();
			Distance.Normalize();
			Force += Distance * ToDesiredDistance;
		}
		
	}

	return -Force;
}


// force away from other agents
FVector UForceManager::GetPoIForce()
{
	FVector2D GroupCenter = FVector2D((Guide->GetPoi()->GetRefPointStanding()).X, (Guide->GetPoi()->GetRefPointStanding()).Y);
	FVector2D Agent = FVector2D(VH->GetActorLocation().X, VH->GetActorLocation().Y);

	FVector2D Direction = GroupCenter - Agent;
	float DesiredLength = Direction.Size() - 220.0f;
		//Direction.Size() - SocialGroup->GroupRadius;
	Direction.Normalize();
	return FVector(Direction, 0) * DesiredLength;

}


TArray<AVirtualHuman*> UForceManager::GetVHs()
{
	
	for (TActorIterator<AVirtualHuman> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
	{
		AVirtualHuman* ACharacter = *CharacterItr;
		VirtualHumans.Add(ACharacter);
	}
	return VirtualHumans;

}


UVHGuide* UForceManager::GetGuide()
{
	TArray<AVirtualHuman*> VirtualHumanArray;
	for (TActorIterator<AVirtualHuman> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
	{
		AVirtualHuman* Character = *CharacterItr;
		VirtualHumanArray.Add(Character);
	}

	for (int i = 0; i < VirtualHumanArray.Num(); i++)
	{
		Guide = Cast<UVHGuide>(VirtualHumanArray[i]->GetComponentByClass(UVHGuide::StaticClass()));
		if (Guide)
		{
			return Guide;
		}
	}
	return Guide;
}