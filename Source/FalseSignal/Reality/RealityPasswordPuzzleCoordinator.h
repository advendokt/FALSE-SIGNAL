// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealityPasswordPuzzleCoordinator.generated.h"

class ARealityGateDoor;

DECLARE_MULTICAST_DELEGATE(FOnPasswordChanged);

UCLASS()
class FALSESIGNAL_API ARealityPasswordPuzzleCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ARealityPasswordPuzzleCoordinator();

	bool SubmitPassword(const FString& SubmittedPassword, AActor* Interactor);

	bool IsCompleted() const { return bCompleted; }
	const FString& GetExpectedPassword() const { return ExpectedPassword; }
	FOnPasswordChanged& OnPasswordChanged() { return OnPasswordChangedDelegate; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Password Puzzle", meta = (ClampMin = 1, ClampMax = 8))
	int32 PasswordLength = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Password Puzzle")
	bool bRandomizePassword = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Password Puzzle")
	FString ManualPassword;

	UPROPERTY(ReplicatedUsing = OnRep_ExpectedPassword, VisibleInstanceOnly, BlueprintReadOnly, Category = "Password Puzzle")
	FString ExpectedPassword;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Password Puzzle")
	TObjectPtr<ARealityGateDoor> Gate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Password Puzzle")
	bool bCompleted = false;

	UFUNCTION()
	void OnRep_ExpectedPassword();

private:
	void GenerateExpectedPassword_Server();
	bool SanitizeManualPassword(FString& InOutPassword) const;
	bool IsNumericPassword(const FString& Password) const;
	void NotifyPasswordChanged();

	FOnPasswordChanged OnPasswordChangedDelegate;
};
