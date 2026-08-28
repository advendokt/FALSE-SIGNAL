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
class UMaterialInstanceDynamic;
class USoundBase;

UCLASS()
class FALSESIGNAL_API ARealitySequenceButton : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealitySequenceButton();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SymbolMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Button")
	ERealityPuzzleSymbol SymbolId = ERealityPuzzleSymbol::Triangle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Button")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sequence Button")
	FRealityPuzzleSymbolVisualSet SymbolVisuals;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Sequence Button")
	TObjectPtr<ARealitySequencePuzzleCoordinator> Coordinator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.0, Units = "cm"))
	float PressOffset = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.01, Units = "s"))
	float ReturnDelay = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Color")
	FName ColorParameterName = TEXT("TintColor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Color")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Color")
	FLinearColor ErrorColor = FLinearColor(1.0f, 0.1f, 0.1f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Color")
	FLinearColor SuccessColor = FLinearColor(0.1f, 1.0f, 0.1f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.05, Units = "s"))
	float ErrorFeedbackDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback", meta = (ClampMin = 0.05, Units = "s"))
	float SuccessFeedbackDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Audio")
	TObjectPtr<USoundBase> ErrorFeedbackSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Audio")
	TObjectPtr<USoundBase> SuccessFeedbackSound;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayPressFeedback();
	void MulticastPlayPressFeedback_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayErrorFeedback();
	void MulticastPlayErrorFeedback_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlaySuccessFeedback();
	void MulticastPlaySuccessFeedback_Implementation();

	void HandlePressFeedbackReturn();
	void HandleFeedbackColorRestore();
	void ApplyLocalPresentation();
	void RefreshSymbolMesh();
	void InitializeFeedbackMaterials();
	void ApplyFeedbackColor(const FLinearColor& Color);
	void PlayFeedbackSound(USoundBase* SoundToPlay) const;

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();

	FVector InitialButtonRelativeLocation = FVector::ZeroVector;
	FTimerHandle PressFeedbackReturnTimerHandle;
	FTimerHandle FeedbackColorRestoreTimerHandle;
	TObjectPtr<UMaterialInstanceDynamic> ButtonMeshDynamicMaterial;
	TObjectPtr<UMaterialInstanceDynamic> SymbolMeshDynamicMaterial;
	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	FDelegateHandle RealityProfileChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;

public:
	void TriggerErrorFeedback();
	void TriggerSuccessFeedback();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
