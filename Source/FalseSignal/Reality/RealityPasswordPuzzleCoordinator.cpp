// Copyright Epic Games, Inc. All Rights Reserved.

#include "Reality/RealityPasswordPuzzleCoordinator.h"
#include "FalseSignal.h"
#include "Net/UnrealNetwork.h"
#include "Reality/RealityGateDoor.h"

ARealityPasswordPuzzleCoordinator::ARealityPasswordPuzzleCoordinator()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ARealityPasswordPuzzleCoordinator::BeginPlay()
{
	Super::BeginPlay();

	bCompleted = false;

	if (HasAuthority())
	{
		GenerateExpectedPassword_Server();
		NotifyPasswordChanged();
	}
}

void ARealityPasswordPuzzleCoordinator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARealityPasswordPuzzleCoordinator, ExpectedPassword);
}

void ARealityPasswordPuzzleCoordinator::OnRep_ExpectedPassword()
{
	NotifyPasswordChanged();
}

bool ARealityPasswordPuzzleCoordinator::SubmitPassword(const FString& SubmittedPassword, AActor* Interactor)
{
	if (!HasAuthority())
	{
		return false;
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Submission received on %s from %s. Input=%s"), *GetNameSafe(this), *GetNameSafe(Interactor), *SubmittedPassword);
#endif

	if (bCompleted)
	{
		return false;
	}

	if (ExpectedPassword.IsEmpty())
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[PasswordPuzzle] Password rejected on %s: expected password is empty"), *GetNameSafe(this));
#endif
		return false;
	}

	if (!IsNumericPassword(SubmittedPassword))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Password rejected on %s: non-numeric input"), *GetNameSafe(this));
#endif
		return false;
	}

	if (SubmittedPassword.Len() != ExpectedPassword.Len())
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Password rejected on %s: wrong length %d expected %d"), *GetNameSafe(this), SubmittedPassword.Len(), ExpectedPassword.Len());
#endif
		return false;
	}

	if (SubmittedPassword != ExpectedPassword)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Password rejected on %s"), *GetNameSafe(this));
#endif
		return false;
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Password accepted on %s"), *GetNameSafe(this));
#endif

	bCompleted = true;

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Puzzle completed on %s. Opening gate=%s"), *GetNameSafe(this), *GetNameSafe(Gate));
#endif

	if (IsValid(Gate))
	{
		Gate->OpenGate();
	}

	return true;
}

void ARealityPasswordPuzzleCoordinator::GenerateExpectedPassword_Server()
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 EffectiveLength = FMath::Clamp(PasswordLength, 1, 8);
	if (PasswordLength != EffectiveLength)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[PasswordPuzzle] PasswordLength %d on %s out of range. Clamped to %d"), PasswordLength, *GetNameSafe(this), EffectiveLength);
#endif
	}

	if (bRandomizePassword)
	{
		ExpectedPassword.Reset();
		for (int32 Index = 0; Index < EffectiveLength; ++Index)
		{
			const int32 Digit = FMath::RandRange(0, 9);
			ExpectedPassword.AppendChar(TCHAR('0' + Digit));
		}

#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Generated password on %s: %s"), *GetNameSafe(this), *ExpectedPassword);
#endif
		return;
	}

	FString SanitizedManualPassword = ManualPassword;
	if (!SanitizeManualPassword(SanitizedManualPassword))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[PasswordPuzzle] ManualPassword invalid on %s. Using zero fallback length %d"), *GetNameSafe(this), EffectiveLength);
#endif
		ExpectedPassword = FString::ChrN(EffectiveLength, TCHAR('0'));
	}
	else
	{
		if (SanitizedManualPassword.Len() > EffectiveLength)
		{
			SanitizedManualPassword = SanitizedManualPassword.Left(EffectiveLength);
		}
		else if (SanitizedManualPassword.Len() < EffectiveLength)
		{
			const int32 MissingDigits = EffectiveLength - SanitizedManualPassword.Len();
			SanitizedManualPassword += FString::ChrN(MissingDigits, TCHAR('0'));
		}

		ExpectedPassword = SanitizedManualPassword;
	}

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[PasswordPuzzle] Generated password on %s: %s"), *GetNameSafe(this), *ExpectedPassword);
#endif
}

bool ARealityPasswordPuzzleCoordinator::SanitizeManualPassword(FString& InOutPassword) const
{
	InOutPassword.TrimStartAndEndInline();

	if (InOutPassword.IsEmpty())
	{
		return false;
	}

	for (int32 Index = InOutPassword.Len() - 1; Index >= 0; --Index)
	{
		if (!FChar::IsDigit(InOutPassword[Index]))
		{
			InOutPassword.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}

	return !InOutPassword.IsEmpty();
}

bool ARealityPasswordPuzzleCoordinator::IsNumericPassword(const FString& Password) const
{
	if (Password.IsEmpty())
	{
		return false;
	}

	for (const TCHAR Character : Password)
	{
		if (!FChar::IsDigit(Character))
		{
			return false;
		}
	}

	return true;
}

void ARealityPasswordPuzzleCoordinator::NotifyPasswordChanged()
{
	OnPasswordChangedDelegate.Broadcast();
}
