// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityPasswordCluePanel.h"
#include "FalseSignal.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Reality/RealityPasswordPuzzleCoordinator.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityPasswordCluePanel::ARealityPasswordCluePanel()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(SceneRoot);
	PanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PanelMesh->SetRelativeScale3D(FVector(0.05f, 1.2f, 1.0f));

	PasswordText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PasswordText"));
	PasswordText->SetupAttachment(SceneRoot);
	PasswordText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PasswordText->SetHorizontalAlignment(EHTA_Center);
	PasswordText->SetVerticalAlignment(EVRTA_TextCenter);
	PasswordText->SetRelativeLocation(FVector(3.0f, 0.0f, 40.0f));
	PasswordText->SetWorldSize(40.0f);
	PasswordText->SetText(FText::FromString(TEXT("")));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PanelMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		PanelMesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void ARealityPasswordCluePanel::BeginPlay()
{
	Super::BeginPlay();

	TryBindToCoordinator();
	TryBindToLocalRealityState();
	RefreshPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityPasswordCluePanel::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityPasswordCluePanel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindCoordinator();
	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityPasswordCluePanel::RefreshPresentation()
{
	RefreshPasswordText();

	EFalseSignalRealityProfile LocalProfile = EFalseSignalRealityProfile::Unassigned;
	if (!BoundLocalPlayerState.IsValid())
	{
		TryBindToLocalRealityState();
	}
	if (BoundLocalPlayerState.IsValid())
	{
		LocalProfile = BoundLocalPlayerState->GetRealityProfile();
	}

	const bool bShowClue = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality && BoundCoordinator.IsValid();

	if (PanelMesh)
	{
		PanelMesh->SetVisibility(bShowClue, true);
	}

	if (PasswordText)
	{
		PasswordText->SetVisibility(bShowClue, true);
	}
}

void ARealityPasswordCluePanel::RefreshPasswordText()
{
	if (!PasswordText)
	{
		return;
	}

	if (!BoundCoordinator.IsValid() && !TryBindToCoordinator())
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[PasswordPuzzle] Clue panel %s has no valid Coordinator assigned"), *GetNameSafe(this));
#endif
		PasswordText->SetText(FText::FromString(TEXT("")));
		return;
	}

	PasswordText->SetText(FText::FromString(BoundCoordinator->GetExpectedPassword()));
}

void ARealityPasswordCluePanel::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	RefreshPresentation();
}

void ARealityPasswordCluePanel::HandlePresentationRetry()
{
	const bool bBoundReality = TryBindToLocalRealityState();
	const bool bBoundCoordinatorNow = TryBindToCoordinator();
	if (bBoundReality || bBoundCoordinatorNow)
	{
		RefreshPresentation();
	}

	const bool bRealityResolved = BoundLocalPlayerState.IsValid() && BoundLocalPlayerState->GetRealityProfile() != EFalseSignalRealityProfile::Unassigned;
	const bool bCoordinatorResolved = BoundCoordinator.IsValid();
	if (bRealityResolved && bCoordinatorResolved && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}
}

bool ARealityPasswordCluePanel::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityPasswordCluePanel::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityPasswordCluePanel::ResolveLocalPlayerState() const
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

void ARealityPasswordCluePanel::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}

bool ARealityPasswordCluePanel::TryBindToCoordinator()
{
	ARealityPasswordPuzzleCoordinator* LocalCoordinator = Coordinator.Get();
	if (LocalCoordinator == BoundCoordinator.Get())
	{
		return BoundCoordinator.IsValid();
	}

	UnbindCoordinator();

	if (!IsValid(LocalCoordinator))
	{
		return false;
	}

	CoordinatorPasswordChangedHandle = LocalCoordinator->OnPasswordChanged().AddUObject(this, &ARealityPasswordCluePanel::HandleCoordinatorPasswordChanged);
	BoundCoordinator = LocalCoordinator;
	return true;
}

void ARealityPasswordCluePanel::UnbindCoordinator()
{
	if (BoundCoordinator.IsValid() && CoordinatorPasswordChangedHandle.IsValid())
	{
		BoundCoordinator->OnPasswordChanged().Remove(CoordinatorPasswordChangedHandle);
	}

	CoordinatorPasswordChangedHandle.Reset();
	BoundCoordinator.Reset();
}

void ARealityPasswordCluePanel::HandleCoordinatorPasswordChanged()
{
	RefreshPresentation();
}
