// Fill out your copyright notice in the Description page of Project Settings.

#include "VHGuide.h"
#include "Malika.h"
#include "Omar.h"
#include "Vivian.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "VHAnimInstance.h"
#include "VHGazing.h"
#include "Engine.h"
#include "AIController.h"
#include "AtPoI.h"
#include "ToPoI.h"
#include "MeetingPoint.h"
#include "AWaypoint.h"
#include "Goodbye.h"
#include "ApproachUser.h"
#include "Helper/CharacterPluginLogging.h"
#include "Utility/VirtualRealityUtilities.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "SocialGroups/VHSocialGroups.h"




UVHGuide::UVHGuide()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentPoIIterator = 0;
	UserMaxSpeed = 150.0f;
	ZValue = 0;
	bTurnToUser = false;
	bGreetUser = true;
	CurrentYaw = 0.0f;
	CurrentWaypoint = 0;
	GuideVH = Cast<ACharacter>(GetOuter());
	SetGuideDialogues();
	//<<<<UserQuestions>>>>
	QuestionOne = CreateDefaultSubobject<UTextRenderComponent>("QuestionOne");
	QuestionTwo = CreateDefaultSubobject<UTextRenderComponent>("QuestionTwo");
	QuestionThree = CreateDefaultSubobject<UTextRenderComponent>("QuestionThree");
}

void UVHGuide::BeginPlay()
{
	Super::BeginPlay();
	if (!Init()) return;
	SetSpeed(VH, 100);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAWaypoint::StaticClass(), Waypoints);
	//ParticipantActor = (UGameplayStatics::GetActorOfClass(GetWorld(), AVHParticipant::StaticClass()));
	//Participant = Cast<AVHParticipant>(ParticipantActor);
	Followers = GetParticipants();
	CurrentPoI = PointsOfInterest[CurrentPoIIterator];
	VRPawn->PawnMovement->MaxSpeed = UserMaxSpeed;
	//*******************************************************
	// create state objects
	//*******************************************************
	AtPoI = NewObject<UAtPoI>(this);
	ToPoI = NewObject<UToPoI>(this);
	ApproachUser = NewObject<UApproachUser>(this);
	MeetingPoint = NewObject<UMeetingPoint>(this);
	Goodbye = NewObject<UGoodbye>(this);
	ChangeState(MeetingPoint);
	State = MeetingPoint;
	Montages = LoadAnimationMontages();
	PreLoadFaceAnimationFiles();
	if (Montages.Num() < 1)
	{
		VH_ERROR("[ASocialGroup::BeginPlay] Could not load guide montages.\n")
			return;
	}
	bDesktopMode = UVirtualRealityUtilities::IsDesktopMode();
	AVirtualRealityPawn* LocalVRPawn = VRPawn;
	auto SetupTextRender = [LocalVRPawn](bool bVR, UTextRenderComponent* TextRender, AActor* Owner, FVector offset, EHorizTextAligment Alignment, FColor Color)
	{
		if (bVR)
		{
			TextRender->AttachToComponent(LocalVRPawn->LeftHand, FAttachmentTransformRules::KeepRelativeTransform);
			TextRender->SetRelativeLocation(offset);
			TextRender->SetWorldSize(3);
			TextRender->SetRelativeRotation(FRotator(90, 180, 0));
		}
		else
		{
			TextRender->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			TextRender->SetRelativeLocation(offset);
			TextRender->SetWorldSize(8);
		}
		TextRender->HorizontalAlignment = Alignment;
		TextRender->SetTextRenderColor(Color);
		TextRender->SetHiddenInGame(true);
	};
	FString QuestionOneString = "Question 1?";
	FString QuestionTwoString = "Question 2?";
	FString QuestionThreeString = "Question 3?";
	const FText QuestionOneText = FText::FromString(QuestionOneString);
	const FText QuestionTwoText = FText::FromString(QuestionTwoString);
	const FText QuestionThreeText = FText::FromString(QuestionThreeString);
	QuestionOne->SetText(QuestionOneText);
	QuestionTwo->SetText(QuestionTwoText);
	QuestionThree->SetText(QuestionThreeText);
	if (bDesktopMode)
	{
		SetupTextRender(false, QuestionOne, GetOwner(), FVector(0, -20, 70), EHorizTextAligment::EHTA_Left, FColor::Green);
		SetupTextRender(false, QuestionTwo, GetOwner(), FVector(0, 20, 70), EHorizTextAligment::EHTA_Right, FColor::Red);
		SetupTextRender(false, QuestionThree, GetOwner(), FVector(0, 0, 90), EHorizTextAligment::EHTA_Center, FColor::Blue);
	}
	else
	{
		SetupTextRender(true, QuestionOne, GetOwner(), FVector(0, 15, 0), EHorizTextAligment::EHTA_Left, FColor::Green);
		SetupTextRender(true, QuestionTwo, GetOwner(), FVector(0, -15, 0), EHorizTextAligment::EHTA_Right, FColor::Red);
		SetupTextRender(true, QuestionThree, GetOwner(), FVector(5, 0, 0), EHorizTextAligment::EHTA_Center, FColor::Blue);
	}
	//AllowUserInput();
}


// Called every frame
void UVHGuide::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//Track distance to user
	UserLocation = VRPawn->Head->GetComponentLocation();
	FVector ToUser = UserLocation - VHPawn->GetActorLocation();
	float DistanceToUser = ToUser.Size();
	//Angle to User to later on check if user is walking next to us 
	GuideToUserAngle = FVector::DotProduct(VH->GetActorForwardVector(), VRPawn->GetActorLocation() - VH->GetActorLocation());
	GuideToUserAngle = FMath::Acos(GuideToUserAngle);
	GuideToUserAngle = FMath::RadiansToDegrees(GuideToUserAngle);
	//Track length between gazing and greeting
	GazeTimeElapsed += DeltaTime;
	GreetTimeElapsed += DeltaTime;
	//check distance to user
	auto PathToUser = UNavigationSystemV1::FindPathToLocationSynchronously(
		GetWorld(), VH->GetActorLocation(), UserLocation);
	//*******************************************************
	// User walks away from guide
	//*******************************************************
	if (PathToUser->GetPathLength() > 1000 && (State == ToPoI))
	{
		// provide current state and location, to know where to get back to
		LocationBeforeInterrupt = VHPawn->GetActorLocation();
		StateBeforeInterruption = State;
		State->OnInterruptionBegin();
	}
	State->GazeModel(DeltaTime);
	State->Tick(DeltaTime);

	if (VH->GetVelocity().Size() <= 2.0 && (AtPoI->PoIState == WaitForUser || AtPoI->PoIState == AtPoi))
	{
		TurnTo(CurrentPoI->GetActorLocation(), 60.0f, 5.0f, DeltaTime);
	}
	//Check how long the user is walking near the guide
	if (DistanceToUser < 180 && GuideToUserAngle <= PeripheralVisionAngle && ToPoI->ToPoIState == FollowPositions)
	{
		UserWalkingNextToGuide += DeltaTime;
	}
	//If user is walking near the guide for longer than 3f or if user is near the guide and looking at him
	if (DistanceToUser < 180.0f && UserOrientedToGuide() && ToPoI->ToPoIState == FollowPositions || UserWalkingNextToGuide > 1.0f && ToPoI->ToPoIState == FollowPositions)
	{
		//Guide looks at user
		if (bLookAtUser)
		{
			State->GazeState = EGazeState::ToUser;
			FacialExpressComp->SelectedEmotion = Happiness;
			bLookAtUser = false;
		}
		//Play a dialogue "Hi, it's a very beautiful city, right?"
		if (bGreetUser)
		{
			if (!this->SpeechComp->bIsSpeaking)
			{
				this->SpeechComp->PlayDialogue(this->GuideDialogues.UserFallsBehind, 0.1f, false);
				GreetTimeElapsed = 0.0f;
				bGreetUser = false;
			}
		}
	}
	//5f pass and then the guide can look at the user again
	if (GazeTimeElapsed > 5.0f)
	{
		bLookAtUser = true;
		GazeTimeElapsed = 0.0f;
	}
	//30f pass and the guide can talk to the user again
	if (GreetTimeElapsed > 150.0f)
	{
		bGreetUser = true;
		GreetTimeElapsed = 0.0f;
	}

	if (bUserCanAskQuestions)
	{
		if (bDesktopMode)
		{
			const auto ORot = UKismetMathLibrary::FindLookAtRotation(QuestionOne->GetComponentLocation(), UserLocation);
			const auto TRot = UKismetMathLibrary::FindLookAtRotation(QuestionTwo->GetComponentLocation(), UserLocation);
			const auto ThRot = UKismetMathLibrary::FindLookAtRotation(QuestionThree->GetComponentLocation(), UserLocation);
			QuestionOne->SetWorldRotation(ORot);
			QuestionTwo->SetWorldRotation(TRot);
			QuestionThree->SetWorldRotation(ThRot);
		}
	}
}

void UVHGuide::AllowUserInput()
{
	bUserCanAskQuestions = true;
	QuestionOne->SetHiddenInGame(false);
	QuestionTwo->SetHiddenInGame(false);
	QuestionThree->SetHiddenInGame(false);
}


void UVHGuide::DisableUserInput()
{
	bUserCanAskQuestions = false;
	QuestionOne->SetHiddenInGame(true);
	QuestionTwo->SetHiddenInGame(true);
	QuestionThree->SetHiddenInGame(true);
}


void UVHGuide::ChangeState(TScriptInterface<IStates> NewState)
{
	if (NewState)
	{
		State = NewState;
		State->OnStateChange();		
	}
	else
	{
		VH_ERROR("[VHGuide]: NewState is not valid\n");
	}
}


/////////////////////////////
//<<<<Points of interest>>>>
////////////////////////////
void UVHGuide::SetPoi(APoI* Poi)
{
	if (IsValid(Poi))
	{
		CurrentPoI = Poi;
		//Val2D.PoiStandingPos = FVector2D(Poi->GetRefPointStanding().X, Poi->GetRefPointStanding().Y);
	}
}


//Probably call when we are already at a poi and want to set the next one
void UVHGuide::NextPoI()
{
	CurrentPoIIterator++;
	if (CurrentPoIIterator >= PointsOfInterest.Num())
	{
		CurrentPoIIterator = -1; // we are through with all PoI
		ChangeState(Goodbye);
		//
	}
	else
	{
		CurrentPoI = PointsOfInterest[CurrentPoIIterator];
		ChangeState(ToPoI);
		//Val2D.PoiStandingPos = FVector2D(CurrentPoi->GetRefPointStanding().X, CurrentPoi->GetRefPointStanding().Y);
	}

}




void UVHGuide::PrintError(FString Msg)
{
	VH_ERROR("[VHGuide]: %s \n", *Msg);
}

void UVHGuide::PrintDebug(FString Msg)
{
	VH_DEBUG("[VHGuide]: %s \n", *Msg);
}

//Initialize all parameters
bool UVHGuide::Init()
{
	
	VH = Cast<AVirtualHuman>(GetOwner());
	if (!VH)
	{
		VH_ERROR("[VHGuide]: Cannot cast to AVirtualHuman.\n")
			return false;
	}
	VHPawn = Cast<APawn>(GetOwner());
	if (!VHPawn)
	{
		VH_ERROR("[VHGuide]: Cannot cast to APawn.\n")
			return false;
	}
	VRPawn = Cast<AVirtualRealityPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!VRPawn)
	{
		VH_ERROR("[VHGuide::BeginPlay] The pawn is not a AVirtualRealityPawn.\n")
			return false;
	}
	AIController = Cast<AGuideAIController>(VHPawn->GetController());
	if (!AIController)
	{
		VH_ERROR("[VHGuide]: cannot cast to AIController.\n")
			return false;
	}
	FaceAnimationInstance = VH->GetFaceAnimInstance();
	if (!FaceAnimationInstance)
	{
		VH_ERROR("[VHGuide]: Face Animation instance is not valid.\n")
			return false;
	}
	BodyAnimationInstance = VH->GetBodyAnimInstance();
	if (!BodyAnimationInstance)
	{
		VH_ERROR("[VHGuide]: Body Animation instance is not valid.\n")
			return false;
	}
	MovementComp = Cast<UCharacterMovementComponent>(VHPawn->GetMovementComponent());
	if (!MovementComp)
	{
		VH_ERROR("[VHGuide]: Movement Component is not vaild.\n")
			return false;
	}
	GazingComp = VH->FindComponentByClass<UVHGazing>();
	if (!GazingComp)
	{
		VH_ERROR("[VHGuide]: Gazing Component is not vaild.\n")
			return false;
	}
	SpeechComp = VH->FindComponentByClass<UVHSpeech>();
	if (!SpeechComp)
	{
		VH_ERROR("[VHGuide]: Dialogue Component is not vaild.\n")
			return false;
	}
	FaceAnimComp = VH->FindComponentByClass<UVHFaceAnimation>();
	if (!FaceAnimComp)
	{
		VH_ERROR("[VHGuide]: UVHFaceAnimation Component is not valid.\n")
			return false;
	}
	PointingComp = GetOwner()->FindComponentByClass<UVHPointing>();
	if (!PointingComp)
	{
		VH_ERROR("[VHGuide]: Pointing Component is not valid\n.")
			return false;
	}
	AudioComp = GetOwner()->FindComponentByClass<UVAAudiofileSourceComponent>();
	if (!AudioComp)
	{
		VH_ERROR("[VHGuide]: AudiofileSource Component is not valid\n.")
			return false;
	}
	FacialExpressComp = VH->FindComponentByClass<UVHFacialExpressions>();
	if (!FacialExpressComp)
	{
		VH_ERROR("[VHGuide]: UVHFacialExpressions Component is not valid.\n")
			return false;
	}
	return true;
}



// force away from other agents
FVector UVHGuide::GetProximityForce()
{
	FVector Force = FVector::ZeroVector;

	FVector CurrentAgent = VHPawn->GetActorLocation();
	FVector OtherAgent = VRPawn->GetActorLocation();
	FVector Distance = OtherAgent - CurrentAgent;

	if (Distance.Size() < PersonalDistance && Distance.Size() > 0)
	{
		float ToDesiredDistance = PersonalDistance - Distance.Size();
		Distance.Normalize();
		Force += Distance * ToDesiredDistance;
	}

	for (int i = 0; i < Followers.Num(); i++)
	{
		CurrentAgent = VHPawn->GetActorLocation();
		OtherAgent = Followers[i]->GetActorLocation();
		Distance = OtherAgent - CurrentAgent;

		if (Distance.Size() < PersonalDistance && Distance.Size() > 0)
		{
			float ToDesiredDistance = PersonalDistance - Distance.Size();
			Distance.Normalize();
			Force += Distance * ToDesiredDistance;
		}

	}
	return -Force;
}


void UVHGuide::TurnTo(FVector Location, float StartThreshold, float EndThreshold, float DeltaTime)
{
	// do not turn if moving
	if (VH->GetVelocity().Size() > 2)
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
		VH->AddActorWorldRotation(FRotator(0, CurrentYaw, 0));
	}
}




TArray<AVirtualHuman*> UVHGuide::GetParticipants()
{
	AVirtualHuman* Malika = Cast<AVirtualHuman>(UGameplayStatics::GetActorOfClass(GetWorld(), AMalika::StaticClass()));
	Participants.Add(Malika);
	AVirtualHuman* Vivian = Cast<AVirtualHuman>(UGameplayStatics::GetActorOfClass(GetWorld(), AVivian::StaticClass()));
	Participants.Add(Vivian);
	AVirtualHuman* Omar = Cast<AVirtualHuman>(UGameplayStatics::GetActorOfClass(GetWorld(), AOmar::StaticClass()));
	Participants.Add(Omar);
	return Participants;
}


//Set the speed of the guide
void UVHGuide::SetSpeed(AVirtualHuman* VirtualH, float Speed)
{
	MovementComp = Cast<UCharacterMovementComponent>(VirtualH->GetMovementComponent());
	MovementComp->MaxWalkSpeed = Speed;
}



void UVHGuide::MoveToWaypoint()
{
	if (CurrentWaypoint <= Waypoints.Num())
	{
		for (AActor* AWaypoint : Waypoints)
		{
			AAWaypoint* WaypointItr = Cast<AAWaypoint>(AWaypoint);
			if (WaypointItr)
			{
				if (WaypointItr->GetWaypointOrder() == CurrentWaypoint)
				{
					AIController->MoveToActor(WaypointItr, 5.0f, false);
					CurrentWaypoint++;
					break;
				}
			}
		}
	}
}


void UVHGuide::SetGuideDialogues()
{

	FDialogueUtterance UserNoticed;
	UserNoticed.Audiofile = FString("/GroupNavigation/Dialogues/ComeJoinUs/ComeJoinUs.wav");
	UserNoticed.FaceAnimFile = FString("/GroupNavigation/Dialogues/ComeJoinUs/ComeJoinUs.csv");
	GuideDialogues.Noticed = UserNoticed;

	FDialogueUtterance UserGreeting;
	UserGreeting.Audiofile = FString("/GroupNavigation/Dialogues/Welcome/Hi_Welcome.wav");  
	UserGreeting.FaceAnimFile = FString("/GroupNavigation/Dialogues/Welcome/Hi_Welcome.csv");
	GuideDialogues.Greeting = UserGreeting;

	FDialogueUtterance UserBehind;
	UserBehind.Audiofile = FString("/GroupNavigation/Dialogues/UserFallsBehind/Hey_OverHere.wav");  
	UserBehind.FaceAnimFile = FString("/GroupNavigation/Dialogues/UserFallsBehind/Hey_OverHere.csv");
	GuideDialogues.UserFallsBehind = UserBehind;


	FDialogueUtterance GuideGoodbye;
	GuideGoodbye.Audiofile = FString("/GroupNavigation/Dialogues/Goodbye/Goodbye.wav");
	GuideGoodbye.FaceAnimFile = FString("/GroupNavigation/Dialogues/Goodbye/Goodbye.csv");
	GuideDialogues.Goodbye = GuideGoodbye;


}

void UVHGuide::PreLoadFaceAnimationFiles()
{
	FaceAnimComp->LoadAnimationFile(GuideDialogues.Greeting.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(GuideDialogues.Noticed.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(GuideDialogues.UserFallsBehind.FaceAnimFile);
	FaceAnimComp->LoadAnimationFile(GuideDialogues.Goodbye.FaceAnimFile);
	for (auto Poi : PointsOfInterest)
	{
		VH_DEBUG("Load Animation Files.")
		FaceAnimComp->LoadAnimationFile(Poi->Dialogue.ApproachMon.FaceAnimFile);
		FaceAnimComp->LoadAnimationFile(Poi->Dialogue.Monologue.FaceAnimFile);

		for (auto RT : Poi->ResponseTriggers)
		{
			FaceAnimComp->LoadAnimationFile(RT->Dialogue.FaceAnimFile);
		}
	}
}


TArray<UAnimMontage*> UVHGuide::LoadAnimationMontages() const
{
	TArray<UAnimMontage*> VHMontages;
	FString Path1 = "";
	FString Path2 = "";

	Path1 = "/CharacterPlugin/Animations/Waving/MH";
	if (!FPaths::ValidatePath(Path1))
	{
		VH_WARN("Path: %s is not valid", *Path1)
	}

	Path2 = "/CharacterPlugin/Animations/Talking/MH";
	if (!FPaths::ValidatePath(Path1))
	{
		VH_WARN("Path: %s is not valid", *Path1)
	}

	TArray<UObject*> Assets;
	EngineUtils::FindOrLoadAssetsByPath(Path1, Assets, EngineUtils::ATL_Regular);
	EngineUtils::FindOrLoadAssetsByPath(Path2, Assets, EngineUtils::ATL_Regular);


	for (UObject* Asset : Assets)
	{
		UAnimMontage* Anim = Cast<UAnimMontage>(Asset);
		if (Anim)
		{
			VHMontages.Add(Anim);
		}
		else
		{
			VH_WARN("Could not find a valid UAnimInstance at %s", *Path1)
		}
	}

	return VHMontages;
}

bool UVHGuide::UserOrientedToGuide()
{
	FVector UserForward = VRPawn->Head->GetForwardVector();
	FVector UserToGuide = UserLocation - VH->GetActorLocation();
	FVector2D UserForwardXY = FVector2D(UserForward.X, UserForward.Y);
	FVector2D UserToGuideXY = FVector2D(UserToGuide.X, UserToGuide.Y);

	UserForwardXY.Normalize();
	UserToGuideXY.Normalize();

	float Angle = FMath::Abs(FVector2D::DotProduct(UserForwardXY, UserToGuideXY));
	Angle = FMath::Acos(Angle);
	Angle = FMath::RadiansToDegrees(Angle);
	return Angle < 20.0f;
}
