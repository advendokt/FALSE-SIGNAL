// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "RealityPuzzleSymbolTypes.generated.h"

UENUM(BlueprintType)
enum class ERealityPuzzleSymbol : uint8
{
	Triangle UMETA(DisplayName = "Triangle"),
	Circle UMETA(DisplayName = "Circle"),
	Square UMETA(DisplayName = "Square"),
	Cross UMETA(DisplayName = "Cross")
};
