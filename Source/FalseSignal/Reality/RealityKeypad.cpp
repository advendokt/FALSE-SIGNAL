// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityKeypad.h"
#include "FalseSignal.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "FalseSignalPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "Reality/RealityPasswordPuzzleCoordinator.h"

ARealityKeypad::ARealityKeypad()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	KeypadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadMesh"));
	KeypadMesh->SetupAttachment(SceneRoot);
	KeypadMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KeypadMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	KeypadMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	KeypadMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	KeypadMesh->SetRelativeScale3D(FVector(0.4f, 0.8f, 1.0f));

	ScreenText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScreenText"));
	ScreenText->SetupAttachment(SceneRoot);
	ScreenText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScreenText->SetHorizontalAlignment(EHTA_Center);
	ScreenText->SetVerticalAlignment(EVRTA_TextCenter);
	ScreenText->SetWorldSize(24.0f);
	ScreenText->SetRelativeLocation(FVector(10.0f, 0.0f, 55.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		KeypadMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		KeypadMesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void ARealityKeypad::BeginPlay()
{
	Super::BeginPlay();
	ApplyScreenState(ERealityKeypadScreenState::Ready);
}

bool ARealityKeypad::SubmitPassword_Server(const FString& SubmittedPassword, AActor* Interactor)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValid(Coordinator))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[PasswordPuzzle] Keypad %s rejected submission: missing Coordinator"), *GetNameSafe(this));
#endif
		MulticastSetScreenState(ERealityKeypadScreenState::Denied);
		return false;
	}

	const bool bAccepted = Coordinator->SubmitPassword(SubmittedPassword, Interactor);
	MulticastSetScreenState(bAccepted ? ERealityKeypadScreenState::Granted : ERealityKeypadScreenState::Denied);
	return bAccepted;
}

void ARealityKeypad::MulticastSetScreenState_Implementation(ERealityKeypadScreenState NewState)
{
	ApplyScreenState(NewState);
}

void ARealityKeypad::ApplyScreenState(ERealityKeypadScreenState NewState)
{
	if (!ScreenText)
	{
		return;
	}

	switch (NewState)
	{
	case ERealityKeypadScreenState::Ready:
		ScreenText->SetText(ReadyText);
		break;
	case ERealityKeypadScreenState::Denied:
		ScreenText->SetText(DeniedText);
		break;
	case ERealityKeypadScreenState::Granted:
		ScreenText->SetText(GrantedText);
		break;
	default:
		ScreenText->SetText(ReadyText);
		break;
	}
}

bool ARealityKeypad::CanInteract_Implementation(AActor* Interactor) const
{
	if (!IsValid(Coordinator) || RequiredReality == EFalseSignalRealityProfile::Unassigned)
	{
		return false;
	}

	return !Coordinator->IsCompleted();
}

void ARealityKeypad::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}

	MulticastSetScreenState(ERealityKeypadScreenState::Ready);
}

bool ARealityKeypad::IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const
{
	return RequiredReality != EFalseSignalRealityProfile::Unassigned && RealityProfile == RequiredReality;
}
