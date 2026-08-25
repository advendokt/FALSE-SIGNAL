// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityTestActor.generated.h"

class AFalseSignalPlayerState;
class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class FALSESIGNAL_API ARealityTestActor : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealityTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WallMesh;

	UPROPERTY(ReplicatedUsing = OnRep_Activated, VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FLinearColor ActivatedColor = FLinearColor(0.1f, 0.8f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FLinearColor InactiveColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DoorDynamicMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> WallDynamicMaterial;

	UFUNCTION()
	void OnRep_Activated();

	void ApplyActivationVisualState();

private:
	void HandleLocalRealityProfileChanged(EFalseSignalRealityProfile NewProfile);
	void HandlePresentationRetry();
	void UpdateLocalPresentation();
	bool TryBindToLocalRealityState();
	AFalseSignalPlayerState* ResolveLocalPlayerState() const;
	void UnbindLocalRealityState();

	TWeakObjectPtr<AFalseSignalPlayerState> BoundLocalPlayerState;
	FDelegateHandle RealityProfileChangedHandle;
	FTimerHandle PresentationRetryTimerHandle;

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
