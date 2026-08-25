// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionComponent.h"
#include "FalseSignal.h"
#include "FalseSignalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AFalseSignalCharacter>(GetOwner());
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCurrentInteractable();
}

AActor* UInteractionComponent::GetCurrentInteractableActor() const
{
	return Cast<AActor>(CurrentInteractable.GetObject());
}

void UInteractionComponent::UpdateCurrentInteractable()
{
	AActor* PreviousTargetActor = GetCurrentInteractableActor();
	AActor* HitActor = nullptr;
	UPrimitiveComponent* HitComponent = nullptr;
	bool bHitActorImplementsInteractable = false;
	AActor* NewTargetActor = nullptr;

	if (!OwnerCharacter.IsValid())
	{
		ClearCurrentInteractable();
	}
	else
	{
		UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCameraComponent();

		if (!IsValid(Camera))
		{
			ClearCurrentInteractable();
		}
		else
		{
			const FVector TraceStart = Camera->GetComponentLocation();
			const FVector TraceEnd = TraceStart + (Camera->GetForwardVector() * InteractionDistance);

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwnerCharacter.Get());

			if (!GetWorld() || !GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
			{
				ClearCurrentInteractable();
			}
			else
			{
				HitActor = HitResult.GetActor();
				HitComponent = HitResult.GetComponent();
				bHitActorImplementsInteractable = IsValid(HitActor) && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass());

				if (!bHitActorImplementsInteractable)
				{
					ClearCurrentInteractable();
				}
				else
				{
					CurrentInteractable.SetObject(HitActor);
					CurrentInteractable.SetInterface(Cast<IInteractable>(HitActor));
					NewTargetActor = HitActor;
				}
			}
		}
	}

	if (!IsValid(NewTargetActor))
	{
		NewTargetActor = GetCurrentInteractableActor();
	}

#if !(UE_BUILD_SHIPPING)
	static TMap<const UInteractionComponent*, TWeakObjectPtr<AActor>> LastHitActorByComponent;
	static TMap<const UInteractionComponent*, TWeakObjectPtr<UPrimitiveComponent>> LastHitComponentByComponent;

	const AActor* LastHitActor = LastHitActorByComponent.FindRef(this).Get();
	const UPrimitiveComponent* LastHitComponent = LastHitComponentByComponent.FindRef(this).Get();

	const bool bTraceChanged = LastHitActor != HitActor || LastHitComponent != HitComponent;
	const bool bTargetChanged = PreviousTargetActor != NewTargetActor;

	if (bTraceChanged || bTargetChanged)
	{
		UE_LOG(LogFalseSignal, Log, TEXT("[InteractionDebug] HitActor=%s HitComponent=%s ImplementsInteractable=%s CurrentTarget=%s"),
			*GetNameSafe(HitActor),
			*GetNameSafe(HitComponent),
			bHitActorImplementsInteractable ? TEXT("true") : TEXT("false"),
			*GetNameSafe(NewTargetActor));
	}

	LastHitActorByComponent.FindOrAdd(this) = HitActor;
	LastHitComponentByComponent.FindOrAdd(this) = HitComponent;
#endif
}

void UInteractionComponent::ClearCurrentInteractable()
{
	CurrentInteractable.SetObject(nullptr);
	CurrentInteractable.SetInterface(nullptr);
}
