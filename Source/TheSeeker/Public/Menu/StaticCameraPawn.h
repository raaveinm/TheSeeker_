// Created by Kirill "Raaveinm"

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StaticCameraPawn.generated.h"

UCLASS()
class THESEEKER_API AStaticCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AStaticCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
