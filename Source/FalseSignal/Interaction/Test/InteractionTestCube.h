// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "InteractionTestCube.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Simple test interactable actor for Phase 2 prototype.
 * Toggles color when interacted with.
 */
UCLASS()
class FALSESIGNAL_API AInteractionTestCube : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteractionTestCube();

protected:
	/** Root mesh for the test cube */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	/** Visual state toggle */
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bIsActive = false;

	/** Color used when active */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FLinearColor ActiveColor = FLinearColor(0.1f, 0.8f, 0.2f, 1.0f);

	/** Color used when inactive */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FLinearColor InactiveColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);

	/** Dynamic material used to drive visible color changes */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Applies current color based on active state */
	void ApplyVisualState();

	UFUNCTION()
	void OnRep_IsActive();

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
};
