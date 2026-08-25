// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityAwareInteractable.generated.h"

UINTERFACE(BlueprintType)
class URealityAwareInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional reality authorization contract for interactables.
 * Implement only on actors that have reality-specific interaction access rules.
 */
class FALSESIGNAL_API IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	/** Returns whether interaction is authorized for the provided authoritative reality profile */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Reality")
	bool IsInteractionAllowedForReality(EFalseSignalRealityProfile RealityProfile) const;
};
