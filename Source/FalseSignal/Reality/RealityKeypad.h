// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "Reality/RealityProfileTypes.h"
#include "RealityKeypad.generated.h"

class ARealityPasswordPuzzleCoordinator;
class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UENUM(BlueprintType)
enum class ERealityKeypadScreenState : uint8
{
	Ready,
	Denied,
	Granted
};

UCLASS()
class FALSESIGNAL_API ARealityKeypad : public AActor, public IInteractable, public IRealityAwareInteractable
{
	GENERATED_BODY()

public:
	ARealityKeypad();

	bool SubmitPassword_Server(const FString& SubmittedPassword, AActor* Interactor);
	float GetMaxSubmitDistance() const { return MaxSubmitDistance; }
	ARealityPasswordPuzzleCoordinator* GetCoordinator() const { return Coordinator.Get(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeypadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> ScreenText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad")
	EFalseSignalRealityProfile RequiredReality = EFalseSignalRealityProfile::RealityB;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Keypad")
	TObjectPtr<ARealityPasswordPuzzleCoordinator> Coordinator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad", meta = (ClampMin = 50.0, ClampMax = 1000.0, Units = "cm"))
	float MaxSubmitDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad")
	FText ReadyText = FText::FromString(TEXT("READY"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad")
	FText DeniedText = FText::FromString(TEXT("DENIED"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad")
	FText GrantedText = FText::FromString(TEXT("GRANTED"));

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSetScreenState(ERealityKeypadScreenState NewState);
	void MulticastSetScreenState_Implementation(ERealityKeypadScreenState NewState);

	void ApplyScreenState(ERealityKeypadScreenState NewState);

public:
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool IsInteractionAllowedForReality_Implementation(EFalseSignalRealityProfile RealityProfile) const override;
};
