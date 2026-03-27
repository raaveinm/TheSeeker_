//
// Created by raaveinm on 2/17/26.
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnitCharacter.generated.h"

///////////////////////////////////////////////
// Slave character
///////////////////////////////////////////////

UCLASS()
class THESEEKER_API AUnitCharacter : public ACharacter {
	GENERATED_BODY()

public:
	AUnitCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
