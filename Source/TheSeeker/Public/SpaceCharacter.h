//
// Created by raaveinm on 3/30/26.
//

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SpaceCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class UCapsuleComponent;
class USkeletalMeshComponent;
class USpringArmComponent;

UCLASS()
class THESEEKER_API ASpaceCharacter : public APawn
{
	GENERATED_BODY()

public:
	ASpaceCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	
	/////////////////////////////////////////////
	// Components
	/////////////////////////////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;
	
	/////////////////////////////////////////////
	// Input
	/////////////////////////////////////////////
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_MoveX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_MoveZ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_TurnYaw;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Interact;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Look;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_SwapShoulder;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Pitch;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_RotationX;

	/////////////////////////////////////////////
	// Physics
	/////////////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight Dynamics")
	float THRUSTER_FORCE;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight Dynamics")
	float TORQUE_FORCE;
	
	/////////////////////////////////////////////
	// Camera Adjustments
	/////////////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Adjust")
	float ARM_LENGTH = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Adjust")
	float LAG_SPEED = 3.f;
	
	/////////////////////////////////////////////
	// VFX/SFX State
	/////////////////////////////////////////////
	UFUNCTION(BlueprintImplementableEvent, Category="VFX")
	void OnThrusterStateChange (bool isActive);
	
	/////////////////////////////////////////////
	// Blueprint Realization
	/////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void InteractClicked();

	/////////////////////////////////////////////
	// Interaction
	/////////////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 BaseDamage = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionScatter = 50.f;

	static const FName TAG_DAMAGEABLE;
	static const FName TAG_INTERACTABLE;
	
private:
	// movement handling
	void MoveX(const FInputActionValue& ia_x);
	void MoveZ(const FInputActionValue& ia_z);
	void TurnYaw(const FInputActionValue& ia_yaw);
	void RotationX(const FInputActionValue& ia_x);
	void CameraRotation(const FInputActionValue& ia_look);
	void SwapShoulder();
	void Pitch(const FInputActionValue& ia_pitch);
	
	// Thruster state observer
	bool isThrustingEnd;
	
	FVector currentLinearInput;
	float currentYawInput;
	float currentXRotationInput;
	float currentPitchInput;
	float shoulderPosition = 125.f;
};
