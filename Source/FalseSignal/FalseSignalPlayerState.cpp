// Copyright Epic Games, Inc. All Rights Reserved.

#include "FalseSignalPlayerState.h"
#include "Net/UnrealNetwork.h"

AFalseSignalPlayerState::AFalseSignalPlayerState()
{
	bReplicates = true;
}

void AFalseSignalPlayerState::SetRealityProfile(EFalseSignalRealityProfile NewProfile)
{
	if (!HasAuthority() || RealityProfile == NewProfile)
	{
		return;
	}

	RealityProfile = NewProfile;
	NotifyRealityProfileChanged();
}

void AFalseSignalPlayerState::OnRep_RealityProfile()
{
	NotifyRealityProfileChanged();
}

void AFalseSignalPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFalseSignalPlayerState, RealityProfile);
}

void AFalseSignalPlayerState::NotifyRealityProfileChanged()
{
	OnRealityProfileChangedDelegate.Broadcast(RealityProfile);
}
