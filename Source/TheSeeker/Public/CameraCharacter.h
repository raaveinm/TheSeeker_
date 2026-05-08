//
// Created by raaveinm on 2/17/26.
//

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "CameraCharacter.generated.h"

class AUnitCharacter;
class ALevelControl;

UCLASS()
class THESEEKER_API ACameraCharacter : public ACharacter {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACameraCharacter();
	
	virtual void PossessedBy(AController* NewController) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	///////////////////////////////////////////////
	// Camera Control
	///////////////////////////////////////////////
	
	// camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Camera")
	class UCameraComponent* TopDownCamera;
	
	// const
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MIN_ZOOM = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float MAX_ZOOM = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float SPEED = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float LENGTH = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float PITCH = -60.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float FOV = 90.f;
	
	///////////////////////////////////////////////
	// Controlled unit linking
	///////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, Category = "RTS Control")
	void SetControlledUnit(AUnitCharacter* unit) { controlledUnit = unit; }
	
	UFUNCTION(BlueprintPure, Category = "RTS Control")
	AUnitCharacter* GetControlledUnit() const { return controlledUnit; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Control")
	AUnitCharacter* controlledUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Control")
	class UInputAction* CommandAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Trace")
	float TraceDistanceAbove = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Trace")
	float TraceDistanceBelow = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Trace")
	float TraceFrequency = 0.1f; // Seconds between traces

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Trace")
	float HeightInterpSpeed = 5.f;

	static const FName TAG_RTS_CAMERA;

	void Command(const FInputActionValue& val);

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Levels Control")
	float AutoFloorCooldown = 1.5f;

private:
	float LastManualFloorTime = -100.f;
	float LastTraceTime = 0.f;
	float TargetZ = 0.f;
	void InteractInput(const FInputActionValue& val);

protected:
	///////////////////////////////////////////////
	// WSAD movement
	///////////////////////////////////////////////
	
	// mapping context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	// movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* MoveAction;
	
	// zoom
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ZoomAction;
	
	// floor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* FloorUpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* FloorDownAction;
	

	// handling movement & zoom
	void Move(const struct FInputActionValue& val);
	void Zoom(const struct FInputActionValue& val);
	
	///////////////////////////////////////////////
	// Level Manager
	///////////////////////////////////////////////
	
	UPROPERTY()
	ALevelControl* LevelManager;
	void ChangeFloorUp(const FInputActionValue& val);
	void ChangeFloorDown(const FInputActionValue& val);
	
	///////////////////////////////////////////////
	// Interaction
	///////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* InteractAction;
	
	UFUNCTION(BlueprintNativeEvent, Category="Interaction")
	void OnInteract(const FInputActionValue& val, FIntVector TargetCoords);
	virtual void OnInteract_Implementation(const FInputActionValue& val, FIntVector TargetCoords);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
