// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealitySequencePuzzleCoordinator.h"
#include "FalseSignal.h"
#include "Net/UnrealNetwork.h"
#include "Reality/RealityGateDoor.h"
#include "Reality/RealitySequenceButton.h"

ARealitySequencePuzzleCoordinator::ARealitySequencePuzzleCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AvailableSymbols =
	{
		ERealityPuzzleSymbol::Triangle,
		ERealityPuzzleSymbol::Circle,
		ERealityPuzzleSymbol::Square,
		ERealityPuzzleSymbol::Cross
	};
}

void ARealitySequencePuzzleCoordinator::BeginPlay()
{
	Super::BeginPlay();

	CurrentInput.Reset();
	bCompleted = false;

	if (HasAuthority())
	{
		if (bRandomizeSequence)
		{
			GenerateExpectedSequence_Server();
		}

		NotifySequenceChanged();
	}
}

void ARealitySequencePuzzleCoordinator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealitySequencePuzzleCoordinator, ExpectedSequence);
}

void ARealitySequencePuzzleCoordinator::OnRep_ExpectedSequence()
{
	NotifySequenceChanged();
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

	if (CurrentInput.Num() >= ExpectedSequence.Num())
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] Input buffer exceeded expected length on %s. Resetting input"), *GetNameSafe(this));
#endif
		CurrentInput.Reset();
	}

	CurrentInput.Add(PressedSymbol);

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Symbol received on %s: %s"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(PressedSymbol));
	UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Input progress on %s: %d/%d"),
		*GetNameSafe(this),
		CurrentInput.Num(),
		ExpectedSequence.Num());
#endif

	if (CurrentInput.Num() < ExpectedSequence.Num())
	{
		return;
	}

	bool bMatches = true;
	for (int32 Index = 0; Index < ExpectedSequence.Num(); ++Index)
	{
		if (CurrentInput[Index] != ExpectedSequence[Index])
		{
			bMatches = false;
			break;
		}
	}

	if (bMatches)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Full sequence accepted on %s"), *GetNameSafe(this));
#endif

		bCompleted = true;

#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Puzzle completed on %s. Opening gate=%s"), *GetNameSafe(this), *GetNameSafe(Gate));
#endif

		if (IsValid(Gate))
		{
			Gate->OpenGate();
		}

		for (ARealitySequenceButton* SequenceButton : SequenceButtons)
		{
			if (IsValid(SequenceButton))
			{
				SequenceButton->TriggerSuccessFeedback();
			}
		}

#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Success feedback triggered on %s for %d buttons"), *GetNameSafe(this), SequenceButtons.Num());
#endif

		CurrentInput.Reset();
		return;
	}

	for (ARealitySequenceButton* SequenceButton : SequenceButtons)
	{
		if (IsValid(SequenceButton))
		{
			SequenceButton->TriggerErrorFeedback();
		}
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Error feedback triggered on %s for %d buttons"), *GetNameSafe(this), SequenceButtons.Num());
#endif

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[SequencePuzzle] Full sequence rejected on %s. Clearing input"), *GetNameSafe(this));
#endif

	CurrentInput.Reset();
}

void ARealitySequencePuzzleCoordinator::GenerateExpectedSequence_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	int32 EffectiveSequenceLength = SequenceLength;
	if (EffectiveSequenceLength <= 0)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] SequenceLength <= 0 on %s. Clamping to 1"), *GetNameSafe(this));
#endif
		EffectiveSequenceLength = 1;
	}

	TArray<ERealityPuzzleSymbol> CandidateSymbols = AvailableSymbols;
	if (!bAllowDuplicateSymbols)
	{
		TSet<ERealityPuzzleSymbol> UniqueSet;
		for (ERealityPuzzleSymbol Symbol : AvailableSymbols)
		{
			UniqueSet.Add(Symbol);
		}
		CandidateSymbols = UniqueSet.Array();
	}

	if (CandidateSymbols.Num() <= 0)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] AvailableSymbols is empty on %s. Puzzle remains inactive"), *GetNameSafe(this));
#endif
		ExpectedSequence.Reset();
		return;
	}

	if (!bAllowDuplicateSymbols && EffectiveSequenceLength > CandidateSymbols.Num())
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[SequencePuzzle] SequenceLength %d exceeds unique AvailableSymbols %d on %s. Clamping"), EffectiveSequenceLength, CandidateSymbols.Num(), *GetNameSafe(this));
#endif
		EffectiveSequenceLength = CandidateSymbols.Num();
	}

	ExpectedSequence.Reset();

	if (bAllowDuplicateSymbols)
	{
		for (int32 Index = 0; Index < EffectiveSequenceLength; ++Index)
		{
			const int32 RandomIndex = FMath::RandRange(0, CandidateSymbols.Num() - 1);
			ExpectedSequence.Add(CandidateSymbols[RandomIndex]);
		}
		return;
	}

	TArray<ERealityPuzzleSymbol> UniquePool = CandidateSymbols;
	for (int32 Index = 0; Index < EffectiveSequenceLength; ++Index)
	{
		if (UniquePool.Num() <= 0)
		{
			break;
		}

		const int32 RandomIndex = FMath::RandRange(0, UniquePool.Num() - 1);
		ExpectedSequence.Add(UniquePool[RandomIndex]);
		UniquePool.RemoveAtSwap(RandomIndex, 1, EAllowShrinking::No);
	}
}

void ARealitySequencePuzzleCoordinator::NotifySequenceChanged()
{
	OnSequenceChangedDelegate.Broadcast();
}
