//
// Created by raaveinm on 3/30/26.
//

#include "SpaceCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASpaceCharacter::ASpaceCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	
	// Root capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->InitCapsuleSize(45.f, 90.f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	
	// Mesh
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	// Camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = ARM_LENGTH;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = LAG_SPEED;
	SpringArmComponent->bUsePawnControlRotation = false; 
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	// Physics
	CapsuleComponent->SetSimulatePhysics(true);
	CapsuleComponent->SetEnableGravity(false);
	CapsuleComponent->SetLinearDamping(2.f); 
	CapsuleComponent->SetAngularDamping(3.f);
	
	// Defaults
	THRUSTER_FORCE = 80000.f;
	TORQUE_FORCE = 550000.f;
	isThrustingEnd = false;
	currentLinearInput = FVector::ZeroVector;
	currentYawInput = 0.f;
}

void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool isThrusting = !currentLinearInput.IsNearlyZero() || FMath::Abs(currentYawInput) > .01f;
	
	if (isThrusting) {
		FVector thrustDirection = GetActorForwardVector() * currentLinearInput.X + GetActorUpVector() * currentLinearInput.Z;
		thrustDirection.Normalize();
		CapsuleComponent->AddForce(thrustDirection * THRUSTER_FORCE);
		
		FVector torque = GetActorUpVector() * currentYawInput * TORQUE_FORCE;
		CapsuleComponent->AddTorqueInRadians(torque);
	}
	
	if (isThrusting && !isThrustingEnd)
	{
		OnThrusterStateChange(isThrusting);
		isThrustingEnd = isThrusting;
	}
	
	currentLinearInput = FVector::ZeroVector;
	currentYawInput = 0.f;
}

/////////////////////////////////////////////
// Control Binding
/////////////////////////////////////////////

void ASpaceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController) return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* IA_Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IA_Subsystem) return;

	if (MappingContext) {
		IA_Subsystem->AddMappingContext(MappingContext, 0);
	}

	UEnhancedInputComponent* IA_Component = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IA_Component) return;

	IA_Component->BindAction(IA_MoveX_Backward, ETriggerEvent::Triggered, this, &ASpaceCharacter::MoveX_Backward);
	IA_Component->BindAction(IA_MoveX, ETriggerEvent::Triggered, this, &ASpaceCharacter::MoveX);
	IA_Component->BindAction(IA_MoveZ, ETriggerEvent::Triggered, this, &ASpaceCharacter::MoveZ);
	IA_Component->BindAction(IA_TurnYaw, ETriggerEvent::Triggered, this, &ASpaceCharacter::TurnYaw);
	if (IA_Interact) IA_Component->BindAction(IA_Interact, ETriggerEvent::Started, this, &ASpaceCharacter::InteractClicked);
	IA_Component->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASpaceCharacter::CameraRotation);
}

// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::SanitizeFloat(ia_x.Get<float>()));
void ASpaceCharacter::MoveX(const FInputActionValue& ia_x){ currentLinearInput.X += ia_x.Get<float>(); }
void ASpaceCharacter::MoveX_Backward(const FInputActionValue& ia_x){ currentLinearInput.X -= ia_x.Get<float>(); }
void ASpaceCharacter::MoveZ(const FInputActionValue& ia_z) { currentLinearInput.Z += ia_z.Get<float>(); }
void ASpaceCharacter::TurnYaw(const FInputActionValue& ia_yaw) { currentYawInput += ia_yaw.Get<float>(); }

void ASpaceCharacter::CameraRotation(const FInputActionValue& ia_look) {
	FVector2D LookAxisVector = ia_look.Get<FVector2D>();
	FRotator CameraRot(LookAxisVector.Y * -1.0f, LookAxisVector.X, 0.0f);
	SpringArmComponent->AddRelativeRotation(CameraRot);
}

void ASpaceCharacter::InteractClicked() {
	FVector start = CapsuleComponent->GetComponentLocation() + CapsuleComponent->GetUpVector() * 50.f;
	FVector absolute_end = start + CapsuleComponent->GetForwardVector() * 1000.f;
	float scatter_amount = 50.f; 
	
	FVector random_offset(
		FMath::RandRange(-scatter_amount, scatter_amount),
		FMath::RandRange(-scatter_amount, scatter_amount),
		FMath::RandRange(-scatter_amount, scatter_amount)
	);
	
	FVector end = absolute_end + random_offset;
	
	FHitResult hit_result;
	FCollisionQueryParams collision_params;
	collision_params.AddIgnoredActor(this);

	bool hit = GetWorld()->LineTraceSingleByChannel(
		hit_result,
		start,
		end,
		ECC_Visibility,
		collision_params
	);
	
	DrawDebugLine(GetWorld(),start,end, hit? FColor::Blue : FColor::Red, false, 2.f, 0, 1.f);
	
	if (hit) {
		AActor* hit_actor = hit_result.GetActor();
		if (!hit_actor) return;
		
		// logging
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Hit Actor: %s"), *hit_actor->GetName()));
		
		if (hit_actor->ActorHasTag(FName("Damageable"))) {
			// logging
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Tag 'Damageable' - OK!"));
			
			UFunction* DamageFunction = hit_actor->FindFunction(FName("Damage"));
			if (DamageFunction) {
				// logging
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Function 'Damage' - OK! Applied 10 DMG."));
				
				struct { int32 DamageAmount;  } Params;
				Params.DamageAmount = 10;
				hit_actor->ProcessEvent(DamageFunction, &Params);
			} else {
				// logging
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ERROR: Function 'Damage' NOT FOUND on Actor!"));
			}
		} else {
			// logging
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("No 'Damageable' Tag. Trying 'Interact' instead."));
			
			UFunction* InteractFunction = hit_actor->FindFunction(FName("Interact"));
			if (InteractFunction) {
				hit_actor->ProcessEvent(InteractFunction, nullptr);
			}
		}
	}
}
