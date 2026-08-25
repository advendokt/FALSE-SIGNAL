// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityTestActor.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityTestActor::ARealityTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	DoorMesh->SetRelativeScale3D(FVector(0.2f, 1.2f, 2.0f));

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(SceneRoot);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WallMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WallMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	WallMesh->SetRelativeScale3D(FVector(0.4f, 2.0f, 2.2f));

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

void ARealityTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (DoorMesh)
	{
		DoorDynamicMaterial = DoorMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (WallMesh)
	{
		WallDynamicMaterial = WallMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	ApplyActivationVisualState();

	TryBindToLocalRealityState();
	UpdateLocalPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityTestActor::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityTestActor::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	UpdateLocalPresentation();
}

void ARealityTestActor::HandlePresentationRetry()
{
	if (TryBindToLocalRealityState())
	{
		UpdateLocalPresentation();
	}

	if (BoundLocalPlayerState.IsValid() && BoundLocalPlayerState->GetRealityProfile() != EFalseSignalRealityProfile::Unassigned && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}
}

void ARealityTestActor::UpdateLocalPresentation()
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

	const bool bIsRealityA = LocalProfile == EFalseSignalRealityProfile::RealityA;
	const bool bIsRealityB = LocalProfile == EFalseSignalRealityProfile::RealityB;
	const bool bShowDoor = bIsRealityA;
	const bool bShowWall = bIsRealityB;

	if (DoorMesh)
	{
		DoorMesh->SetVisibility(bShowDoor, true);
	}

	if (WallMesh)
	{
		WallMesh->SetVisibility(bShowWall, true);
	}
}

void ARealityTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealityTestActor, bActivated);
}

void ARealityTestActor::OnRep_Activated()
{
	ApplyActivationVisualState();
}

void ARealityTestActor::ApplyActivationVisualState()
{
	const FLinearColor TargetColor = bActivated ? ActivatedColor : InactiveColor;

	if (DoorDynamicMaterial)
	{
		DoorDynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
		DoorDynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TargetColor);
	}

	if (WallDynamicMaterial)
	{
		WallDynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
		WallDynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TargetColor);
	}
}

bool ARealityTestActor::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void ARealityTestActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	bActivated = !bActivated;
	ApplyActivationVisualState();
}

bool ARealityTestActor::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RealityProfile == EFalseSignalRealityProfile::RealityA;
}

bool ARealityTestActor::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityTestActor::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityTestActor::ResolveLocalPlayerState() const
{
	// Prototype-only viewer resolution. This is intentionally isolated so it can be replaced later
	// with a more robust per-viewer perception context system.
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

void ARealityTestActor::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}
