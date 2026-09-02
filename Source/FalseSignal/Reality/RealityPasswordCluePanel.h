// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityPasswordCluePanel.generated.h"

class AFalseSignalPlayerState;
class ARealityPasswordPuzzleCoordinator;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class FALSESIGNAL_API ARealityPasswordCluePanel : public AActor
{
	GENERATED_BODY()

public:
	ARealityPasswordCluePanel();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PanelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> PasswordText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clue")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityA;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Clue")
	TObjectPtr<ARealityPasswordPuzzleCoordinator> Coordinator;

	void RefreshPresentation();
	void RefreshPasswordText();

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();
	bool TryBindToCoordinator();
	void UnbindCoordinator();
	void HandleCoordinatorPasswordChanged();

	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	TWeakObjectPtr<ARealityPasswordPuzzleCoordinator> BoundCoordinator;
	FDelegateHandle RealityProfileChangedHandle;
	FDelegateHandle CoordinatorPasswordChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;
};
