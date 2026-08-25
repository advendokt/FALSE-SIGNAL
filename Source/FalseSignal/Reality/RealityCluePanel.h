// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Reality/RealityProfileTypes.h"
#include "Reality/RealityPuzzleSymbolTypes.h"
#include "RealityCluePanel.generated.h"

class AFalseSignalPlayerState;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

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
	TObjectPtr<UTextRenderComponent> SymbolText0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> SymbolText1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> SymbolText2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> SymbolText3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	TArray<ERealityPuzzleSymbol> ClueSymbols;

	void ApplyLocalPresentation();
	void ApplyClueText();
	FText SymbolToText(ERealityPuzzleSymbol Symbol) const;

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();

	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	FDelegateHandle RealityProfileChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;
};
