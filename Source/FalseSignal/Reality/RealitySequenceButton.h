// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "Reality/RealityProfileTypes.h"
#include "Reality/RealityPuzzleSymbolTypes.h"
#include "RealitySequenceButton.generated.h"

class ARealitySequencePuzzleCoordinator;
class AFalseSignalPlayerState;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class FALSESIGNAL_API ARealitySequenceButton : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealitySequenceButton();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Button")
	ERealityPuzzleSymbol SymbolId = ERealityPuzzleSymbol::Triangle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Button")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityB;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Button")
	TObjectPtr<ARealitySequencePuzzleCoordinator> Coordinator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.0, Units = "cm"))
	float PressOffset = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.01, Units = "s"))
	float ReturnDelay = 0.2f;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayPressFeedback();
	void MulticastPlayPressFeedback_Implementation();

	void HandlePressFeedbackReturn();
	void ApplyLocalPresentation();

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();

	FVector InitialButtonRelativeLocation = FVector::ZeroVector;
	FTimerHandle PressFeedbackReturnTimerHandle;
	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	FDelegateHandle RealityProfileChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
