// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Reality/RealityPuzzleSymbolTypes.h"
#include "RealitySequencePuzzleCoordinator.generated.h"

class ARealityGateDoor;

UCLASS()
class FALSESIGNAL_API ARealitySequencePuzzleCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ARealitySequencePuzzleCoordinator();

	void HandleSymbolPressed(ERealityPuzzleSymbol PressedSymbol);

	bool IsCompleted() const { return bCompleted; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TArray<ERealityPuzzleSymbol> ExpectedSequence;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	TObjectPtr<ARealityGateDoor> Gate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	int32 CurrentStep = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Sequence Puzzle")
	bool bCompleted = false;
};
