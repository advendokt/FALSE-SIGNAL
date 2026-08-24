// Copyright Epic Games, Inc. All Rights Reserved.

#include "FalseSignalGameMode.h"
#include "FalseSignalPlayerState.h"
#include "GameFramework/PlayerController.h"

AFalseSignalGameMode::AFalseSignalGameMode()
{
	PlayerStateClass = AFalseSignalPlayerState::StaticClass();
}

void AFalseSignalGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority() || !IsValid(NewPlayer))
	{
		return;
	}

	AFalseSignalPlayerState* FalseSignalPlayerState = NewPlayer->GetPlayerState<AFalseSignalPlayerState>();
	if (!FalseSignalPlayerState)
	{
		return;
	}

	const EFalseSignalRealityProfile AssignedProfile = (AssignedRealityPlayerCount == 0)
		? EFalseSignalRealityProfile::RealityA
		: EFalseSignalRealityProfile::RealityB;

	FalseSignalPlayerState->SetRealityProfile(AssignedProfile);
	++AssignedRealityPlayerCount;
}
