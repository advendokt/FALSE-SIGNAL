// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityPassage.h"
#include "FalseSignal.h"
#include "FalseSignalCharacter.h"
#include "FalseSignalPlayerState.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityPassage::ARealityPassage()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(SceneRoot);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(SceneRoot);
	InteractionZone->SetBoxExtent(FVector(60.0f, 90.0f, 120.0f));
	InteractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionZone->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	DestinationPoint = CreateDefaultSubobject<USceneComponent>(TEXT("DestinationPoint"));
	DestinationPoint->SetupAttachment(SceneRoot);
	DestinationPoint->SetRelativeLocation(FVector(250.0f, 0.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMesh.Object);
		WallMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		DoorMesh->SetMaterial(0, BaseMaterial.Object);
		WallMesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void ARealityPassage::BeginPlay()
{
	Super::BeginPlay();

	TryBindToLocalRealityState();
	ApplyLocalPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityPassage::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityPassage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityPassage::ApplyLocalPresentation()
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

	const bool bShowDoor = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality;
	const bool bShowWall = !bShowDoor;

	if (DoorMesh)
	{
		DoorMesh->SetVisibility(bShowDoor, true);
	}

	if (WallMesh)
	{
		WallMesh->SetVisibility(bShowWall, true);
	}
}

void ARealityPassage::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	ApplyLocalPresentation();
}

void ARealityPassage::HandlePresentationRetry()
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

bool ARealityPassage::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityPassage::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityPassage::ResolveLocalPlayerState() const
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

void ARealityPassage::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}

bool ARealityPassage::CanInteract_Implementation(AActor* Interactor) const
{
	if (!IsValid(Interactor) || !IsValid(DestinationPoint))
	{
		return false;
	}

	return Cast<AFalseSignalCharacter>(Interactor) != nullptr;
}

void ARealityPassage::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(Interactor))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityPassage] Traversal failed: invalid Interactor on %s"), *GetNameSafe(this));
#endif
		return;
	}

	AFalseSignalCharacter* InteractorCharacter = Cast<AFalseSignalCharacter>(Interactor);
	if (!IsValid(InteractorCharacter))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityPassage] Traversal failed: Interactor %s is not AFalseSignalCharacter on %s"), *GetNameSafe(Interactor), *GetNameSafe(this));
#endif
		return;
	}

	if (!IsValid(DestinationPoint))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityPassage] Traversal failed: invalid DestinationPoint on %s"), *GetNameSafe(this));
#endif
		return;
	}

	const FVector DestinationLocation = DestinationPoint->GetComponentLocation();
	const FRotator DestinationRotation = DestinationPoint->GetComponentRotation();
	const bool bTeleported = InteractorCharacter->TeleportTo(DestinationLocation, DestinationRotation, false, false);

#if !(UE_BUILD_SHIPPING)
	if (bTeleported)
	{
		UE_LOG(LogFalseSignal, Log, TEXT("[RealityPassage] Traversal success Passage=%s Interactor=%s RequiredReality=%s Destination=%s"),
			*GetNameSafe(this),
			*GetNameSafe(InteractorCharacter),
			*UEnum::GetValueAsString(RequiredReality),
			*DestinationLocation.ToCompactString());
	}
	else
	{
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityPassage] Traversal failed: TeleportTo rejected Passage=%s Interactor=%s RequiredReality=%s Destination=%s"),
			*GetNameSafe(this),
			*GetNameSafe(InteractorCharacter),
			*UEnum::GetValueAsString(RequiredReality),
			*DestinationLocation.ToCompactString());
	}
#endif
}

bool ARealityPassage::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RequiredReality != EFalseSignalRealityProfile::Unassigned && RealityProfile == RequiredReality;
}
