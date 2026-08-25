// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealityCoopPuzzleCoordinator.generated.h"

class ARealityPuzzleSwitch;
class ARealityGateDoor;

UCLASS()
class FALSESIGNAL_API ARealityCoopPuzzleCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ARealityCoopPuzzleCoordinator();

	/** Server-side notification from puzzle switches */
	void NotifySwitchActivated(const ARealityPuzzleSwitch* ActivatedSwitch);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ARealityPuzzleSwitch> SwitchA;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ARealityPuzzleSwitch> SwitchB;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ARealityGateDoor> Gate;

private:
	void EvaluatePuzzleState();
};
