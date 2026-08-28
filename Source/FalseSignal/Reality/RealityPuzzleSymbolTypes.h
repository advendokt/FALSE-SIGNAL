// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "RealityPuzzleSymbolTypes.generated.h"

UENUM(BlueprintType)
enum class ERealityPuzzleSymbol : uint8
{
	Triangle UMETA(DisplayName = "Triangle"),
	Circle UMETA(DisplayName = "Circle"),
	Square UMETA(DisplayName = "Square"),
	Cross UMETA(DisplayName = "Cross")
};

USTRUCT(BlueprintType)
struct FRealityPuzzleSymbolVisualSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Symbols")
	TObjectPtr<UStaticMesh> TriangleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Symbols")
	TObjectPtr<UStaticMesh> CircleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Symbols")
	TObjectPtr<UStaticMesh> SquareMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Symbols")
	TObjectPtr<UStaticMesh> CrossMesh = nullptr;

	UStaticMesh* GetMeshForSymbol(ERealityPuzzleSymbol Symbol) const
	{
		switch (Symbol)
		{
		case ERealityPuzzleSymbol::Triangle:
			return TriangleMesh;
		case ERealityPuzzleSymbol::Circle:
			return CircleMesh;
		case ERealityPuzzleSymbol::Square:
			return SquareMesh;
		case ERealityPuzzleSymbol::Cross:
			return CrossMesh;
		default:
			break;
		}

		return nullptr;
	}
};
