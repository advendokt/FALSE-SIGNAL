// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityCoopPuzzleCoordinator.h"
#include "FalseSignal.h"
#include "Reality/RealityGateDoor.h"
#include "Reality/RealityPuzzleSwitch.h"

ARealityCoopPuzzleCoordinator::ARealityCoopPuzzleCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void ARealityCoopPuzzleCoordinator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		EvaluatePuzzleState();
	}
}

void ARealityCoopPuzzleCoordinator::NotifySwitchActivated(const ARealityPuzzleSwitch* ActivatedSwitch)
{
	if (!HasAuthority())
	{
		return;
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityCoopPuzzleCoordinator] NotifySwitchActivated Switch=%s"), *GetNameSafe(ActivatedSwitch));
#endif

	EvaluatePuzzleState();
}

void ARealityCoopPuzzleCoordinator::EvaluatePuzzleState()
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bSwitchAActive = IsValid(SwitchA) && SwitchA->IsActivated();
	const bool bSwitchBActive = IsValid(SwitchB) && SwitchB->IsActivated();

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityCoopPuzzleCoordinator] Evaluate A=%s B=%s"), bSwitchAActive ? TEXT("true") : TEXT("false"), bSwitchBActive ? TEXT("true") : TEXT("false"));
#endif

	if (bSwitchAActive && bSwitchBActive && IsValid(Gate))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[RealityCoopPuzzleCoordinator] Puzzle complete. Opening gate %s"), *GetNameSafe(Gate));
#endif
		Gate->OpenGate();
	}
}
