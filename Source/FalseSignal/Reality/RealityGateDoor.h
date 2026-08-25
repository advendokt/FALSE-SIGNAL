// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityGateDoor.generated.h"

class AFalseSignalPlayerState;
class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class FALSESIGNAL_API ARealityGateDoor : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealityGateDoor();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> SharedBlocker;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, VisibleAnywhere, BlueprintReadOnly, Category = "Gate")
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_IsOpen();

	void ApplyGateState();
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

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
