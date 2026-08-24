// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "RealityProfileTypes.generated.h"

UENUM(BlueprintType)
enum class EFalseSignalRealityProfile : uint8
{
	Unassigned UMETA(DisplayName = "Unassigned"),
	RealityA UMETA(DisplayName = "Reality A"),
	RealityB UMETA(DisplayName = "Reality B")
};
