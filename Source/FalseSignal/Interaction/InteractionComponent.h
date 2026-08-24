// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable.h"
#include "InteractionComponent.generated.h"

class AFalseSignalCharacter;

UCLASS(ClassGroup=(FalseSignal), meta=(BlueprintSpawnableComponent))
class FALSESIGNAL_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** Max interaction distance in cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = 0, Units = "cm"))
	float InteractionDistance = 300.0f;

	/** Currently focused interactable object from camera trace */
	UPROPERTY(Transient)
	TScriptInterface<IInteractable> CurrentInteractable;

	/** Actor that owns this interaction component */
	UPROPERTY(Transient)
	TWeakObjectPtr<AFalseSignalCharacter> OwnerCharacter;

public:
	/** Returns the actor currently focused by the interaction trace */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractableActor() const;

	/** Returns max interaction distance in cm */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractionDistance() const { return InteractionDistance; }

protected:
	/** Updates the current interactable using a line trace from first person camera */
	void UpdateCurrentInteractable();

	/** Clears current interactable reference */
	void ClearCurrentInteractable();
};
