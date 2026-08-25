// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityPuzzleSwitch.generated.h"

class AFalseSignalPlayerState;
class ARealityCoopPuzzleCoordinator;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FALSESIGNAL_API ARealityPuzzleSwitch : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealityPuzzleSwitch();

	bool IsActivated() const { return bActivated; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SwitchMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::Unassigned;

	UPROPERTY(ReplicatedUsing = OnRep_Activated, VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle")
	bool bActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FRotator ActivatedRelativeRotation = FRotator(-35.0f, 0.0f, 0.0f);

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ARealityCoopPuzzleCoordinator> Coordinator;

	UFUNCTION()
	void OnRep_Activated();

	void ApplyActivatedVisualState();
	void ApplyLocalPresentation();

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();

	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	FDelegateHandle RealityProfileChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;
	FRotator InitialSwitchRelativeRotation = FRotator::ZeroRotator;

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
