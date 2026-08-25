// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealitySequenceButton.h"
#include "FalseSignal.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Reality/RealitySequencePuzzleCoordinator.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealitySequenceButton::ARealitySequenceButton()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	ButtonMesh->SetupAttachment(SceneRoot);
	ButtonMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ButtonMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ButtonMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ButtonMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ButtonMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		ButtonMesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void ARealitySequenceButton::BeginPlay()
{
	Super::BeginPlay();

	if (ButtonMesh)
	{
		InitialButtonRelativeLocation = ButtonMesh->GetRelativeLocation();
	}

	TryBindToLocalRealityState();
	ApplyLocalPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealitySequenceButton::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealitySequenceButton::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(PressFeedbackReturnTimerHandle);
	}

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

bool ARealitySequenceButton::CanInteract_Implementation(AActor* Interactor) const
{
	if (!IsValid(Coordinator) || RequiredReality == EFalseSignalRealityProfile::Unassigned)
	{
		return false;
	}

	return !Coordinator->IsCompleted();
}

void ARealitySequenceButton::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}

	Coordinator->HandleSymbolPressed(SymbolId);
	MulticastPlayPressFeedback();
}

bool ARealitySequenceButton::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RequiredReality != EFalseSignalRealityProfile::Unassigned && RealityProfile == RequiredReality;
}

void ARealitySequenceButton::MulticastPlayPressFeedback_Implementation()
{
	if (!ButtonMesh)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PressFeedbackReturnTimerHandle);
	}

	const FVector PressedLocation = InitialButtonRelativeLocation + FVector(-PressOffset, 0.0f, 0.0f);
	ButtonMesh->SetRelativeLocation(PressedLocation);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PressFeedbackReturnTimerHandle, this, &ARealitySequenceButton::HandlePressFeedbackReturn, ReturnDelay, false);
	}
}

void ARealitySequenceButton::HandlePressFeedbackReturn()
{
	if (!ButtonMesh)
	{
		return;
	}

	ButtonMesh->SetRelativeLocation(InitialButtonRelativeLocation);
}

void ARealitySequenceButton::ApplyLocalPresentation()
{
	EFalseSignalRealityProfile LocalProfile = EFalseSignalRealityProfile::Unassigned;

	if (!BoundLocalPlayerState.IsValid())
	{
		TryBindToLocalRealityState();
	}

	if (BoundLocalPlayerState.IsValid())
	{
		LocalProfile = BoundLocalPlayerState->GetRealityProfile();
	}

	const bool bShowButton = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality;
	if (ButtonMesh)
	{
		ButtonMesh->SetVisibility(bShowButton, true);
	}
}

void ARealitySequenceButton::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	ApplyLocalPresentation();
}

void ARealitySequenceButton::HandlePresentationRetry()
{
	if (TryBindToLocalRealityState())
	{
		ApplyLocalPresentation();
	}

	if (BoundLocalPlayerState.IsValid() && BoundLocalPlayerState->GetRealityProfile() != EFalseSignalRealityProfile::Unassigned && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}
}

bool ARealitySequenceButton::TryBindToLocalRealityState()
{
	AFalseSignalPlayerState* LocalPlayerState = ResolveLocalPlayerState();
	if (LocalPlayerState == BoundLocalPlayerState.Get())
	{
		return BoundLocalPlayerState.IsValid();
	}

	UnbindLocalRealityState();

	if (!IsValid(LocalPlayerState))
	{
		return false;
	}

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealitySequenceButton::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealitySequenceButton::ResolveLocalPlayerState() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	APlayerController* LocalPlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(LocalPlayerController))
	{
		return nullptr;
	}

	return LocalPlayerController->GetPlayerState<AFalseSignalPlayerState>();
}

void ARealitySequenceButton::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}
