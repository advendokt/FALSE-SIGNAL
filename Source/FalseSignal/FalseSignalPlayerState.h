// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Reality/RealityProfileTypes.h"
#include "FalseSignalPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRealityProfileChanged, EFalseSignalRealityProfile);

UCLASS()
class FALSESIGNAL_API AFalseSignalPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AFalseSignalPlayerState();

	EFalseSignalRealityProfile GetRealityProfile() const { return RealityProfile; }
	void SetRealityProfile(EFalseSignalRealityProfile NewProfile);

	FOnRealityProfileChanged& OnRealityProfileChanged() { return OnRealityProfileChangedDelegate; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RealityProfile, VisibleAnywhere, BlueprintReadOnly, Category = "Reality")
	EFalseSignalRealityProfile RealityProfile = EFalseSignalRealityProfile::Unassigned;

	UFUNCTION()
	void OnRep_RealityProfile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void NotifyRealityProfileChanged();

	FOnRealityProfileChanged OnRealityProfileChangedDelegate;
};
