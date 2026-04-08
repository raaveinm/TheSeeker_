//
// Created by raaveinm on 2/17/26.
//

#include "UnitCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

AUnitCharacter::AUnitCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 600.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->AvoidanceWeight = .5f;
}

void AUnitCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AUnitCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AUnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
