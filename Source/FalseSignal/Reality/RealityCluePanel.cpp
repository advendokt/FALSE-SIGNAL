// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityCluePanel.h"
#include "FalseSignal.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Reality/RealitySequencePuzzleCoordinator.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityCluePanel::ARealityCluePanel()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	PanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));
	PanelMesh->SetupAttachment(SceneRoot);
	PanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PanelMesh->SetRelativeScale3D(FVector(0.05f, 2.0f, 1.5f));

	SymbolSlot0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot0"));
	SymbolSlot0->SetupAttachment(SceneRoot);
	SymbolSlot0->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot0->SetRelativeLocation(FVector(3.0f, -120.0f, 60.0f));
	SymbolSlot0->SetRelativeScale3D(FVector(0.25f));

	SymbolSlot1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot1"));
	SymbolSlot1->SetupAttachment(SceneRoot);
	SymbolSlot1->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot1->SetRelativeLocation(FVector(3.0f, -72.0f, 60.0f));
	SymbolSlot1->SetRelativeScale3D(FVector(0.25f));

	SymbolSlot2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot2"));
	SymbolSlot2->SetupAttachment(SceneRoot);
	SymbolSlot2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot2->SetRelativeLocation(FVector(3.0f, -24.0f, 60.0f));
	SymbolSlot2->SetRelativeScale3D(FVector(0.25f));

	SymbolSlot3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot3"));
	SymbolSlot3->SetupAttachment(SceneRoot);
	SymbolSlot3->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot3->SetRelativeLocation(FVector(3.0f, 24.0f, 60.0f));
	SymbolSlot3->SetRelativeScale3D(FVector(0.25f));

	SymbolSlot4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot4"));
	SymbolSlot4->SetupAttachment(SceneRoot);
	SymbolSlot4->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot4->SetRelativeLocation(FVector(3.0f, 72.0f, 60.0f));
	SymbolSlot4->SetRelativeScale3D(FVector(0.25f));

	SymbolSlot5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolSlot5"));
	SymbolSlot5->SetupAttachment(SceneRoot);
	SymbolSlot5->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SymbolSlot5->SetRelativeLocation(FVector(3.0f, 120.0f, 60.0f));
	SymbolSlot5->SetRelativeScale3D(FVector(0.25f));

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

void ARealityCluePanel::BeginPlay()
{
	Super::BeginPlay();

	TryBindToCoordinator();
	TryBindToLocalRealityState();
	RefreshPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityCluePanel::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityCluePanel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindCoordinator();
	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityCluePanel::RefreshPresentation()
{
	RefreshSymbolsFromSequence();

	EFalseSignalRealityProfile LocalProfile = EFalseSignalRealityProfile::Unassigned;
	if (!BoundLocalPlayerState.IsValid())
	{
		TryBindToLocalRealityState();
	}
	if (BoundLocalPlayerState.IsValid())
	{
		LocalProfile = BoundLocalPlayerState->GetRealityProfile();
	}

	const bool bShowClue = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality;

	if (PanelMesh)
	{
		PanelMesh->SetVisibility(bShowClue, true);
	}

	const TArray<UStaticMeshComponent*> Slots = { SymbolSlot0, SymbolSlot1, SymbolSlot2, SymbolSlot3, SymbolSlot4, SymbolSlot5 };
	for (UStaticMeshComponent* Slot : Slots)
	{
		const bool bHasMesh = IsValid(Slot) && IsValid(Slot->GetStaticMesh());
		SetSlotVisibility(Slot, bShowClue && bHasMesh);
	}
}

void ARealityCluePanel::RefreshSymbolsFromSequence()
{
	const TArray<ERealityPuzzleSymbol>& SequenceSource = GetCurrentSequenceSource();
	constexpr int32 MaxSlots = 6;

	if (SequenceSource.Num() > MaxSlots)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] CluePanel %s received sequence length %d, showing first %d"), *GetNameSafe(this), SequenceSource.Num(), MaxSlots);
#endif
	}

	const TArray<UStaticMeshComponent*> Slots = { SymbolSlot0, SymbolSlot1, SymbolSlot2, SymbolSlot3, SymbolSlot4, SymbolSlot5 };
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		UStaticMesh* MeshToSet = nullptr;
		if (SequenceSource.IsValidIndex(Index))
		{
			MeshToSet = SymbolVisuals.GetMeshForSymbol(SequenceSource[Index]);
		}
		SetSlotMesh(Slots[Index], MeshToSet);
	}
}

void ARealityCluePanel::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	RefreshPresentation();
}

void ARealityCluePanel::HandlePresentationRetry()
{
	const bool bBoundReality = TryBindToLocalRealityState();
	const bool bBoundCoordinatorLocal = TryBindToCoordinator();
	if (bBoundReality || bBoundCoordinatorLocal)
	{
		RefreshPresentation();
	}

	if (BoundLocalPlayerState.IsValid() && BoundLocalPlayerState->GetRealityProfile() != EFalseSignalRealityProfile::Unassigned && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}
}

bool ARealityCluePanel::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityCluePanel::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityCluePanel::ResolveLocalPlayerState() const
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

void ARealityCluePanel::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}

bool ARealityCluePanel::TryBindToCoordinator()
{
	ARealitySequencePuzzleCoordinator* LocalCoordinator = Coordinator.Get();
	if (LocalCoordinator == BoundCoordinator.Get())
	{
		return BoundCoordinator.IsValid();
	}

	UnbindCoordinator();

	if (!IsValid(LocalCoordinator))
	{
		return false;
	}

	CoordinatorSequenceChangedHandle = LocalCoordinator->OnSequenceChanged().AddUObject(this, &ARealityCluePanel::HandleCoordinatorSequenceChanged);
	BoundCoordinator = LocalCoordinator;
	return true;
}

void ARealityCluePanel::UnbindCoordinator()
{
	if (BoundCoordinator.IsValid() && CoordinatorSequenceChangedHandle.IsValid())
	{
		BoundCoordinator->OnSequenceChanged().Remove(CoordinatorSequenceChangedHandle);
	}

	CoordinatorSequenceChangedHandle.Reset();
	BoundCoordinator.Reset();
}

void ARealityCluePanel::HandleCoordinatorSequenceChanged()
{
	RefreshPresentation();
}

const TArray<ERealityPuzzleSymbol>& ARealityCluePanel::GetCurrentSequenceSource() const
{
	if (BoundCoordinator.IsValid())
	{
		return BoundCoordinator->GetExpectedSequence();
	}

	if (IsValid(Coordinator.Get()))
	{
		return Coordinator->GetExpectedSequence();
	}

	return ClueSymbols;
}

void ARealityCluePanel::SetSlotVisibility(UStaticMeshComponent* Slot, bool bVisible)
{
	if (!Slot)
	{
		return;
	}

	Slot->SetVisibility(bVisible, true);
}

void ARealityCluePanel::SetSlotMesh(UStaticMeshComponent* Slot, UStaticMesh* Mesh)
{
	if (!Slot)
	{
		return;
	}

	Slot->SetStaticMesh(Mesh);
}
