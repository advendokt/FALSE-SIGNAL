// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/Test/InteractionTestCube.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AInteractionTestCube::AInteractionTestCube()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BaseMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, BaseMaterial.Object);
	}
}

void AInteractionTestCube::BeginPlay()
{
	Super::BeginPlay();

	if (Mesh)
	{
		DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	ApplyVisualState();
}

void AInteractionTestCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractionTestCube, bIsActive);
}

void AInteractionTestCube::ApplyVisualState()
{
	if (DynamicMaterial)
	{
		const FLinearColor TargetColor = bIsActive ? ActiveColor : InactiveColor;
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TargetColor);
	}
}

bool AInteractionTestCube::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void AInteractionTestCube::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsActive = !bIsActive;
	ApplyVisualState();
}

void AInteractionTestCube::OnRep_IsActive()
{
	ApplyVisualState();
}
