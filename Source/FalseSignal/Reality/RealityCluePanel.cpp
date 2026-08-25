// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityCluePanel.h"
#include "FalseSignalPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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

	SymbolText0 = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SymbolText0"));
	SymbolText0->SetupAttachment(SceneRoot);
	SymbolText0->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SymbolText0->SetWorldSize(24.0f);
	SymbolText0->SetRelativeLocation(FVector(3.0f, -90.0f, 60.0f));

	SymbolText1 = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SymbolText1"));
	SymbolText1->SetupAttachment(SceneRoot);
	SymbolText1->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SymbolText1->SetWorldSize(24.0f);
	SymbolText1->SetRelativeLocation(FVector(3.0f, -30.0f, 60.0f));

	SymbolText2 = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SymbolText2"));
	SymbolText2->SetupAttachment(SceneRoot);
	SymbolText2->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SymbolText2->SetWorldSize(24.0f);
	SymbolText2->SetRelativeLocation(FVector(3.0f, 30.0f, 60.0f));

	SymbolText3 = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SymbolText3"));
	SymbolText3->SetupAttachment(SceneRoot);
	SymbolText3->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	SymbolText3->SetWorldSize(24.0f);
	SymbolText3->SetRelativeLocation(FVector(3.0f, 90.0f, 60.0f));

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

	ApplyClueText();
	TryBindToLocalRealityState();
	ApplyLocalPresentation();

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

	UnbindLocalRealityState();

	Super::EndPlay(EndPlayReason);
}

void ARealityCluePanel::ApplyLocalPresentation()
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

	const bool bShowClue = RequiredReality != EFalseSignalRealityProfile::Unassigned && LocalProfile == RequiredReality;

	if (PanelMesh)
	{
		PanelMesh->SetVisibility(bShowClue, true);
	}

	if (SymbolText0)
	{
		SymbolText0->SetVisibility(bShowClue, true);
	}
	if (SymbolText1)
	{
		SymbolText1->SetVisibility(bShowClue, true);
	}
	if (SymbolText2)
	{
		SymbolText2->SetVisibility(bShowClue, true);
	}
	if (SymbolText3)
	{
		SymbolText3->SetVisibility(bShowClue, true);
	}
}

void ARealityCluePanel::ApplyClueText()
{
	const int32 NumSymbols = ClueSymbols.Num();

	if (SymbolText0)
	{
		SymbolText0->SetText(NumSymbols > 0 ? SymbolToText(ClueSymbols[0]) : FText::GetEmpty());
	}
	if (SymbolText1)
	{
		SymbolText1->SetText(NumSymbols > 1 ? SymbolToText(ClueSymbols[1]) : FText::GetEmpty());
	}
	if (SymbolText2)
	{
		SymbolText2->SetText(NumSymbols > 2 ? SymbolToText(ClueSymbols[2]) : FText::GetEmpty());
	}
	if (SymbolText3)
	{
		SymbolText3->SetText(NumSymbols > 3 ? SymbolToText(ClueSymbols[3]) : FText::GetEmpty());
	}
}

FText ARealityCluePanel::SymbolToText(ERealityPuzzleSymbol Symbol) const
{
	switch (Symbol)
	{
	case ERealityPuzzleSymbol::Triangle:
		return FText::FromString(TEXT("TRIANGLE"));
	case ERealityPuzzleSymbol::Circle:
		return FText::FromString(TEXT("CIRCLE"));
	case ERealityPuzzleSymbol::Square:
		return FText::FromString(TEXT("SQUARE"));
	case ERealityPuzzleSymbol::Cross:
		return FText::FromString(TEXT("CROSS"));
	default:
		break;
	}

	return FText::FromString(TEXT("UNKNOWN"));
}

void ARealityCluePanel::HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile)
{
	ApplyLocalPresentation();
}

void ARealityCluePanel::HandlePresentationRetry()
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
