// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Reality/RealityPuzzleSymbolTypes.h"
#include "RealitySequencePuzzleCoordinator.generated.h"

class ARealityGateDoor;
class ARealitySequenceButton;

DECLARE_MULTICAST_DELEGATE(FOnSequenceChanged);

UCLASS()
class FALSESIGNAL_API ARealitySequencePuzzleCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ARealitySequencePuzzleCoordinator();

	void HandleSymbolPressed(ERealityPuzzleSymbol PressedSymbol);

	bool IsCompleted() const { return bCompleted; }
	const TArray<ERealityPuzzleSymbol>& GetExpectedSequence() const { return ExpectedSequence; }
	FOnSequenceChanged& OnSequenceChanged() { return OnSequenceChangedDelegate; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_ExpectedSequence, EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TArray<ERealityPuzzleSymbol> ExpectedSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Puzzle")
	bool bRandomizeSequence = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Puzzle", meta = (ClampMin = 1))
	int32 SequenceLength = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Puzzle")
	bool bAllowDuplicateSymbols = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Puzzle")
	TArray<ERealityPuzzleSymbol> AvailableSymbols;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TObjectPtr<ARealityGateDoor> Gate;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TArray<TObjectPtr<ARealitySequenceButton>> SequenceButtons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TArray<ERealityPuzzleSymbol> CurrentInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	bool bCompleted = false;

	UFUNCTION()
	void OnRep_ExpectedSequence();

private:
	void GenerateExpectedSequence_Server();
	void NotifySequenceChanged();

	FOnSequenceChanged OnSequenceChangedDelegate;
};
