// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Reality/RealityProfileTypes.h"
#include "Reality/RealityPuzzleSymbolTypes.h"
#include "RealityCluePanel.generated.h"

class AFalseSignalPlayerState;
class ARealitySequencePuzzleCoordinator;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FALSESIGNAL_API ARealityCluePanel : public AActor
{
	GENERATED_BODY()

public:
	ARealityCluePanel();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PanelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolSlot5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	TArray<ERealityPuzzleSymbol> ClueSymbols;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	FRealityPuzzleSymbolVisualSet SymbolVisuals;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Clue")
	TObjectPtr<ARealitySequencePuzzleCoordinator> Coordinator;

	void RefreshPresentation();
	void RefreshSymbolsFromSequence();

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();
	bool TryBindToCoordinator();
	void UnbindCoordinator();
	void HandleCoordinatorSequenceChanged();
	const TArray<ERealityPuzzleSymbol>& GetCurrentSequenceSource() const;
	void SetSlotVisibility(UStaticMeshComponent* Slot, bool bVisible);
	void SetSlotMesh(UStaticMeshComponent* Slot, UStaticMesh* Mesh);

	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	TWeakObjectPtr<ARealitySequencePuzzleCoordinator> BoundCoordinator;
	FDelegateHandle RealityProfileChangedHandle;
	FDelegateHandle CoordinatorSequenceChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;
};
