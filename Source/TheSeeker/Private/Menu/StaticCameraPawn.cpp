// Created by Kirill "Raaveinm"


#include "Menu/StaticCameraPawn.h"

// Sets default values
AStaticCameraPawn::AStaticCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AStaticCameraPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AStaticCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AStaticCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

