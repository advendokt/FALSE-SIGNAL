// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityGateDoor.h"
#include "FalseSignal.h"
#include "FalseSignalPlayerState.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ARealityGateDoor::ARealityGateDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(SceneRoot);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorMesh->SetRelativeScale3D(FVector(0.2f, 1.2f, 2.0f));

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(SceneRoot);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WallMesh->SetRelativeScale3D(FVector(0.4f, 2.0f, 2.2f));

	SharedBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("SharedBlocker"));
	SharedBlocker->SetupAttachment(SceneRoot);
	SharedBlocker->SetBoxExtent(FVector(50.0f, 120.0f, 160.0f));
	SharedBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SharedBlocker->SetCollisionObjectType(ECC_WorldDynamic);
	SharedBlocker->SetCollisionResponseToAllChannels(ECR_Ignore);
	SharedBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	SharedBlocker->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

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

void ARealityGateDoor::OpenGate()
{
	if (!HasAuthority() || bIsOpen)
	{
		return;
	}

	bIsOpen = true;
	ApplyGateState();

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityGateDoor] OpenGate Actor=%s"), *GetNameSafe(this));
#endif
}

void ARealityGateDoor::BeginPlay()
{
	Super::BeginPlay();

	ApplyGateState();
	TryBindToLocalRealityState();
	ApplyLocalPresentation();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PresentationRetryTimerHandle, this, &ARealityGateDoor::HandlePresentationRetry, 0.25f, true);
	}
}

void ARealityGateDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PresentationRetryTimerHandle);
	}

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityGateDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealityGateDoor, bIsOpen);
}

void ARealityGateDoor::OnRep_IsOpen()
{
	ApplyGateState();
#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Verbose, TEXT("[RealityGateDoor] OnRep_IsOpen Actor=%s bIsOpen=%s"), *GetNameSafe(this), bIsOpen ? TEXT("true") : TEXT("false"));
#endif
}

void ARealityGateDoor::ApplyGateState()
{
	if (SharedBlocker)
	{
		SharedBlocker->SetCollisionEnabled(bIsOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	}

	ApplyLocalPresentation();
}

void ARealityGateDoor::ApplyLocalPresentation()
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

	bool bShowDoor = false;
	bool bShowWall = false;

	if (!bIsOpen)
	{
		bShowDoor = bIsRealityA;
		bShowWall = bIsRealityB;
	}

	if (DoorMesh)
	{
		DoorMesh->SetVisibility(bShowDoor, true);
	}

	if (WallMesh)
	{
		WallMesh->SetVisibility(bShowWall, true);
	}
}

void ARealityGateDoor::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	ApplyLocalPresentation();
}

void ARealityGateDoor::HandlePresentationRetry()
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

bool ARealityGateDoor::TryBindToLocalRealityState()
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

	RealityProfileChangedHandle = LocalPlayerState->OnRealityProfileChanged().AddUObject(this, &ARealityGateDoor::HandleLocalRealityProfileChanged);
	BoundLocalPlayerState = LocalPlayerState;
	return true;
}

AFalseSignalPlayerState* ARealityGateDoor::ResolveLocalPlayerState() const
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

void ARealityGateDoor::UnbindLocalRealityState()
{
	if (BoundLocalPlayerState.IsValid() && RealityProfileChangedHandle.IsValid())
	{
		BoundLocalPlayerState->OnRealityProfileChanged().Remove(RealityProfileChangedHandle);
	}

	RealityProfileChangedHandle.Reset();
	BoundLocalPlayerState.Reset();
}

bool ARealityGateDoor::CanInteract_Implementation(AActor* Interactor) const
{
	return bAllowDirectInteraction && !bIsOpen;
}

void ARealityGateDoor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !bAllowDirectInteraction)
	{
		return;
	}

	OpenGate();

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityGateDoor] Opened by %s on %s"), *GetNameSafe(Interactor), *GetNameSafe(this));
#endif
}

bool ARealityGateDoor::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RealityProfile == EFalseSignalRealityProfile::RealityA;
}
