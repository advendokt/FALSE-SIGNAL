// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityPuzzleSwitch.h"
#include "FalseSignal.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Reality/RealityCoopPuzzleCoordinator.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityPuzzleSwitch::ARealityPuzzleSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));
	SwitchMesh->SetupAttachment(SceneRoot);
	SwitchMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SwitchMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SwitchMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SwitchMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		SwitchMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		SwitchMesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void ARealityPuzzleSwitch::BeginPlay()
{
	Super::BeginPlay();

	if (SwitchMesh)
	{
		InitialSwitchRelativeRotation = SwitchMesh->GetRelativeRotation();
	}

	ApplyActivatedVisualState();
	TryBindToLocalRealityState();
	ApplyLocalPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityPuzzleSwitch::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityPuzzleSwitch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityPuzzleSwitch::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealityPuzzleSwitch, bActivated);
}

void ARealityPuzzleSwitch::OnRep_Activated()
{
	ApplyActivatedVisualState();
}

void ARealityPuzzleSwitch::ApplyActivatedVisualState()
{
	if (!SwitchMesh)
	{
		return;
	}

	const FRotator TargetRotation = bActivated
		? InitialSwitchRelativeRotation + ActivatedRelativeRotation
		: InitialSwitchRelativeRotation;

	SwitchMesh->SetRelativeRotation(TargetRotation);
}

void ARealityPuzzleSwitch::ApplyLocalPresentation()
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

	const bool bVisibleForLocalReality = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality;
	if (SwitchMesh)
	{
		SwitchMesh->SetVisibility(bVisibleForLocalReality, true);
	}
}

void ARealityPuzzleSwitch::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	ApplyLocalPresentation();
}

void ARealityPuzzleSwitch::HandlePresentationRetry()
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

bool ARealityPuzzleSwitch::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityPuzzleSwitch::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityPuzzleSwitch::ResolveLocalPlayerState() const
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

void ARealityPuzzleSwitch::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}

bool ARealityPuzzleSwitch::CanInteract_Implementation(AActor* Interactor) const
{
	return !bActivated;
}

void ARealityPuzzleSwitch::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bActivated)
	{
		return;
	}

	bActivated = true;
	ApplyActivatedVisualState();

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityPuzzleSwitch] Activated Switch=%s RequiredReality=%s"), *GetNameSafe(this), *UEnum::GetValueAsString(RequiredReality));
#endif

	if (IsValid(Coordinator))
	{
		Coordinator->NotifySwitchActivated(this);
	}
}

bool ARealityPuzzleSwitch::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RequiredReality != EFalseSignalRealityProfile::Unassigned && RealityProfile == RequiredReality;
}
