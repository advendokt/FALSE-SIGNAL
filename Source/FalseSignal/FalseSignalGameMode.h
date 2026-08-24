// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FalseSignalGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AFalseSignalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFalseSignalGameMode();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	int32 AssignedRealityPlayerCount = 0;
};



