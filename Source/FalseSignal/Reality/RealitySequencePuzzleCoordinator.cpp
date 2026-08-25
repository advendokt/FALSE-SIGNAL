// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealitySequencePuzzleCoordinator.h"
#include "FalseSignal.h"
#include "Reality/RealityGateDoor.h"

ARealitySequencePuzzleCoordinator::ARealitySequencePuzzleCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void ARealitySequencePuzzleCoordinator::BeginPlay()
{
	Super::BeginPlay();

	CurrentStep = 0;
	bCompleted = false;
}

void ARealitySequencePuzzleCoordinator::HandleSymbolPressed(ERealityPuzzleSymbol PressedSymbol)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bCompleted)
	{
		return;
	}

	if (ExpectedSequence.Num() <= 0)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] Ignored symbol on %s because ExpectedSequence is empty"), *GetNameSafe(this));
#endif
		return;
	}

	if (!ExpectedSequence.IsValidIndex(CurrentStep))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] CurrentStep out of range on %s. Resetting to 0"), *GetNameSafe(this));
#endif
		CurrentStep = 0;
	}

	const ERealityPuzzleSymbol ExpectedSymbol = ExpectedSequence[CurrentStep];
	if (PressedSymbol == ExpectedSymbol)
	{
		++CurrentStep;

#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Correct symbol on %s. Pressed=%s Step=%d/%d"),
			*GetNameSafe(this),
			*UEnum::GetValueAsString(PressedSymbol),
			CurrentStep,
			ExpectedSequence.Num());
#endif

		if (CurrentStep >= ExpectedSequence.Num())
		{
			bCompleted = true;
#if !(UE_BUILD_SHIPPING)
			UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Puzzle completed on %s. Opening gate=%s"), *GetNameSafe(this), *GetNameSafe(Gate));
#endif
			if (IsValid(Gate))
			{
				Gate->OpenGate();
			}
		}

		return;
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Wrong symbol on %s. Pressed=%s Expected=%s. Reset to 0"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(PressedSymbol),
		*UEnum::GetValueAsString(ExpectedSymbol));
#endif

	CurrentStep = 0;
}
