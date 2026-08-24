// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionComponent.h"
#include "FalseSignalCharacter.h"
#include "Camera/CameraComponent.h"
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
	if (!OwnerCharacter.IsValid())
	{
		ClearCurrentInteractable();
		return;
	}

	UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCameraComponent();

	if (!IsValid(Camera))
	{
		ClearCurrentInteractable();
		return;
	}

	const FVector TraceStart = Camera->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (Camera->GetForwardVector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter.Get());

	if (!GetWorld() || !GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		ClearCurrentInteractable();
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		ClearCurrentInteractable();
		return;
	}

	CurrentInteractable.SetObject(HitActor);
	CurrentInteractable.SetInterface(Cast<IInteractable>(HitActor));
}

void UInteractionComponent::ClearCurrentInteractable()
{
	CurrentInteractable.SetObject(nullptr);
	CurrentInteractable.SetInterface(nullptr);
}
