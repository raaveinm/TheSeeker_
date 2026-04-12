// Created by Kirill "Raaveinm"

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StaticCameraPawn.generated.h"

class UCameraComponent;

UCLASS()
class THESEEKER_API AStaticCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AStaticCameraPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	
	/////////////////////////////////////////////
	/// Camera Setup
	/////////////////////////////////////////////
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float MaxYawDeflection = 15.0f;
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float MaxPitchDeflection = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float InterpSpeed = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool CameraFollowEnabled;

private:
	FRotator Rotation;

};
