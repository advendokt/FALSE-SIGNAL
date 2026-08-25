// Copyright Epic Games, Inc. All Rights Reserved.

#include "FalseSignalCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "FalseSignalPlayerState.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/Interactable.h"
#include "Interaction/RealityAwareInteractable.h"
#include "FalseSignal.h"
#include "Engine/World.h"

AFalseSignalCharacter::AFalseSignalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Create the interaction component
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction Component"));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AFalseSignalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFalseSignalCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFalseSignalCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFalseSignalCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFalseSignalCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFalseSignalCharacter::LookInput);

		// Interacting
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFalseSignalCharacter::DoInteract);
		}
	}
	else
	{
		UE_LOG(LogFalseSignal, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFalseSignalCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AFalseSignalCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AFalseSignalCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AFalseSignalCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AFalseSignalCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AFalseSignalCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AFalseSignalCharacter::DoInteract()
{
	if (!InteractionComponent)
	{
		return;
	}

	AActor* TargetActor = InteractionComponent->GetCurrentInteractableActor();
	if (!IsValid(TargetActor))
	{
		return;
	}

	ServerTryInteract(TargetActor);
}

void AFalseSignalCharacter::ServerTryInteract_Implementation(AActor* TargetActor)
{
	if (!ValidateInteractionTarget_Server(TargetActor))
	{
		return;
	}

	IInteractable::Execute_Interact(TargetActor, this);
}

bool AFalseSignalCharacter::ValidateInteractionTarget_Server(AActor* TargetActor)
{
	if (!HasAuthority() || !IsValid(TargetActor))
	{
		return false;
	}

	if (TargetActor->GetWorld() != GetWorld())
	{
		return false;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return false;
	}

	if (!ValidateInteractionReality_Server(TargetActor))
	{
		return false;
	}

	if (!InteractionComponent)
	{
		return false;
	}

	const float MaxDistance = InteractionComponent->GetInteractionDistance();
	if (MaxDistance <= 0.0f)
	{
		return false;
	}

	if (!ValidateInteractionDistance_Server(TargetActor, MaxDistance))
	{
		return false;
	}

	if (!ValidateInteractionLineOfSight_Server(TargetActor, MaxDistance))
	{
		return false;
	}

	if (!IInteractable::Execute_CanInteract(TargetActor, this))
	{
		return false;
	}

	return true;
}

bool AFalseSignalCharacter::ValidateInteractionDistance_Server(const AActor* TargetActor, float MaxDistance)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (!GetServerInteractionView(ViewLocation, ViewDirection))
	{
		return false;
	}

	const float DistanceTolerance = 25.0f;
	const float MaxDistanceWithTolerance = MaxDistance + DistanceTolerance;

	return FVector::DistSquared(ViewLocation, TargetActor->GetActorLocation()) <= FMath::Square(MaxDistanceWithTolerance);
}

bool AFalseSignalCharacter::ValidateInteractionLineOfSight_Server(const AActor* TargetActor, float MaxDistance)
{
	if (!IsValid(TargetActor) || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (!GetServerInteractionView(ViewLocation, ViewDirection))
	{
		return false;
	}

	const float DistanceTolerance = 25.0f;
	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + (ViewDirection * (MaxDistance + DistanceTolerance));

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ServerTryInteractTrace), false);
	QueryParams.AddIgnoredActor(this);
	if (const AController* OwningController = GetController())
	{
		QueryParams.AddIgnoredActor(OwningController);
	}

	if (!GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	return HitResult.GetActor() == TargetActor;
}

bool AFalseSignalCharacter::ValidateInteractionReality_Server(const AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	const bool bIsRealityAware = TargetActor->GetClass()->ImplementsInterface(URealityAwareInteractable::StaticClass());
	if (!bIsRealityAware)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Log, TEXT("[RealityValidation] Player=%s Reality=%s Target=%s Result=ALLOW (TargetNotRealityAware)"),
			*GetNameSafe(this),
			TEXT("N/A"),
			*GetNameSafe(TargetActor));
#endif
		return true;
	}

	const AFalseSignalPlayerState* FalseSignalPlayerState = GetPlayerState<AFalseSignalPlayerState>();
	if (!IsValid(FalseSignalPlayerState))
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityValidation] Player=%s Reality=%s Target=%s Result=DENY (MissingPlayerState)"),
			*GetNameSafe(this),
			TEXT("N/A"),
			*GetNameSafe(TargetActor));
#endif
		return false;
	}

	const EFalseSignalRealityProfile RealityProfile = FalseSignalPlayerState->GetRealityProfile();
	if (RealityProfile == EFalseSignalRealityProfile::Unassigned)
	{
#if !(UE_BUILD_SHIPPING)
		UE_LOG(LogFalseSignal, Warning, TEXT("[RealityValidation] Player=%s Reality=%s Target=%s Result=DENY (UnassignedReality)"),
			*GetNameSafe(this),
			*UEnum::GetValueAsString(RealityProfile),
			*GetNameSafe(TargetActor));
#endif
		return false;
	}

	const bool bAllowed = IRealityAwareInteractable::Execute_IsInteractionAllowedForReality(const_cast<AActor*>(TargetActor), RealityProfile);

#if !(UE_BUILD_SHIPPING)
	UE_LOG(LogFalseSignal, Log, TEXT("[RealityValidation] Player=%s Reality=%s Target=%s Result=%s"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(RealityProfile),
		*GetNameSafe(TargetActor),
		bAllowed ? TEXT("ALLOW") : TEXT("DENY"));
#endif

	return bAllowed;
}

bool AFalseSignalCharacter::GetServerInteractionView(FVector& OutLocation, FVector& OutDirection)
{
	if (const AController* OwningController = GetController())
	{
		FRotator ViewRotation = FRotator::ZeroRotator;
		OwningController->GetPlayerViewPoint(OutLocation, ViewRotation);
		OutDirection = ViewRotation.Vector();
		return true;
	}

	if (const UCameraComponent* Camera = GetFirstPersonCameraComponent())
	{
		OutLocation = Camera->GetComponentLocation();
		OutDirection = Camera->GetForwardVector();
		return true;
	}

	return false;
}
