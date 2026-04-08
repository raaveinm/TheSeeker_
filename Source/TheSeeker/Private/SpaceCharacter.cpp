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
#include "DamageableInterface.h"
#include "InteractableInterface.h"
#include "Kismet/GameplayStatics.h"

const FName ASpaceCharacter::TAG_DAMAGEABLE(TEXT("Damageable"));
const FName ASpaceCharacter::TAG_INTERACTABLE(TEXT("Interactable"));

ASpaceCharacter::ASpaceCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	
	// Root capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetMobility(EComponentMobility::Movable);
	CapsuleComponent->InitCapsuleSize(45.f, 90.f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	
	// Mesh
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	
	// Camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = ARM_LENGTH;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = LAG_SPEED;
	SpringArmComponent->SocketOffset = FVector(-25.f, shoulderPosition, 25.f);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	// Physics
	CapsuleComponent->SetSimulatePhysics(true);
	CapsuleComponent->SetEnableGravity(false);
	CapsuleComponent->SetLinearDamping(2.f); 
	CapsuleComponent->SetAngularDamping(3.f);
	
	// Defaults
	THRUSTER_FORCE = 2500.f;
	TORQUE_FORCE = 15.f;
	isThrustingEnd = false;
	currentLinearInput = FVector::ZeroVector;
	currentYawInput = 0.f;
	currentPitchInput = 0.f;
	currentXRotationInput = 0.f;
}

void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (CapsuleComponent) {
		CapsuleComponent->WakeAllRigidBodies();
	}
}

void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bool isThrusting = !currentLinearInput.IsNearlyZero() 
	|| !FMath::IsNearlyZero(currentYawInput)
	|| !FMath::IsNearlyZero(currentXRotationInput)
	|| !FMath::IsNearlyZero(currentPitchInput);
	
	if (isThrusting) {
		FVector thrustDirection = GetActorForwardVector() * currentLinearInput.X + GetActorUpVector() * currentLinearInput.Z;
		if (!thrustDirection.IsNearlyZero()) {
			thrustDirection.Normalize();
			CapsuleComponent->AddForce(thrustDirection * THRUSTER_FORCE, NAME_None, true);
		}
		
		FVector torque = GetActorUpVector() * currentYawInput * TORQUE_FORCE;
		CapsuleComponent->AddTorqueInRadians(torque, NAME_None, true);
		
		FVector pitchTorque = GetActorRightVector() * currentPitchInput * TORQUE_FORCE;
		CapsuleComponent->AddTorqueInRadians(pitchTorque, NAME_None, true);

		FVector rollTorque = GetActorForwardVector() * currentXRotationInput * TORQUE_FORCE;
		CapsuleComponent->AddTorqueInRadians(rollTorque, NAME_None, true);

		// Debug movement
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Cyan, FString::Printf(TEXT("Thrusting: %s | Force: %.2f"), *thrustDirection.ToString(), THRUSTER_FORCE));
			GEngine->AddOnScreenDebugMessage(2, 0.1f, FColor::Yellow, FString::Printf(TEXT("Simulating: %s | Mass: %.2f"), CapsuleComponent->IsSimulatingPhysics() ? TEXT("YES") : TEXT("NO"), CapsuleComponent->GetMass()));
		}
	}
	
	if (isThrusting && !isThrustingEnd)
	{
		OnThrusterStateChange(isThrusting);
		isThrustingEnd = isThrusting;
	}
	
	float CurrentY = SpringArmComponent->SocketOffset.Y;
	SpringArmComponent->SocketOffset.Y = FMath::FInterpTo(CurrentY, shoulderPosition, DeltaTime, 10.f);

	currentLinearInput = FVector::ZeroVector;
	currentYawInput = 0.f;
	currentPitchInput = 0.f;
	currentXRotationInput = 0.f;
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

	IA_Component->BindAction(IA_MoveX, ETriggerEvent::Triggered, this, &ASpaceCharacter::MoveX);
	IA_Component->BindAction(IA_MoveZ, ETriggerEvent::Triggered, this, &ASpaceCharacter::MoveZ);
	IA_Component->BindAction(IA_TurnYaw, ETriggerEvent::Triggered, this, &ASpaceCharacter::TurnYaw);
	IA_Component->BindAction(IA_RotationX, ETriggerEvent::Triggered, this, &ASpaceCharacter::RotationX);
	IA_Component->BindAction(IA_Pitch, ETriggerEvent::Triggered, this, &ASpaceCharacter::Pitch);
	if (IA_Interact) IA_Component->BindAction(IA_Interact, ETriggerEvent::Started, this, &ASpaceCharacter::InteractClicked);
	IA_Component->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASpaceCharacter::CameraRotation);
	IA_Component->BindAction(IA_SwapShoulder, ETriggerEvent::Started, this, &ASpaceCharacter::SwapShoulder);
}

// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::SanitizeFloat(ia_x.Get<float>()));
void ASpaceCharacter::MoveX(const FInputActionValue& ia_x){ currentLinearInput.X += ia_x.Get<float>(); }
void ASpaceCharacter::MoveZ(const FInputActionValue& ia_z) { currentLinearInput.Z += ia_z.Get<float>(); }
void ASpaceCharacter::TurnYaw(const FInputActionValue& ia_yaw) { currentYawInput += ia_yaw.Get<float>(); }
void ASpaceCharacter::RotationX(const FInputActionValue& ia_x) { currentXRotationInput += ia_x.Get<float>(); }
void ASpaceCharacter::Pitch(const FInputActionValue& ia_pitch){ currentPitchInput += ia_pitch.Get<float>(); }

void ASpaceCharacter::SwapShoulder() { shoulderPosition *= -1.f; }

void ASpaceCharacter::CameraRotation(const FInputActionValue& ia_look) {
	FVector2D LookAxisVector = ia_look.Get<FVector2D>();
	FRotator CameraRot(LookAxisVector.Y * -1.0f, LookAxisVector.X, 0.0f);
	SpringArmComponent->AddRelativeRotation(CameraRot);
}

void ASpaceCharacter::InteractClicked() {
	FVector start = CameraComponent->GetComponentLocation() + CameraComponent->GetUpVector() * 10.f;
	FVector absolute_end = start + CameraComponent->GetForwardVector() * InteractionDistance;
	
	FVector random_offset(
		FMath::RandRange(-InteractionScatter, InteractionScatter),
		FMath::RandRange(-InteractionScatter, InteractionScatter),
		FMath::RandRange(-InteractionScatter, InteractionScatter)
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
	
	DrawDebugLine(GetWorld(),start,end, hit? FColor::Blue : FColor::Red, false, .5f, 0, 1.f);
	
	if (hit) {
		AActor* hit_actor = hit_result.GetActor();
		if (!hit_actor) return;
		
		// logging
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Hit Actor: %s"), *hit_actor->GetName()));
		
		if (hit_actor->ActorHasTag(TAG_DAMAGEABLE) || hit_actor->Implements<UDamageableInterface>()) {
			if (hit_actor->Implements<UDamageableInterface>()) {
				IDamageableInterface::Execute_Damage(hit_actor, BaseDamage);
			} else {
				// Fallback to Unreal's native damage system
				UGameplayStatics::ApplyDamage(hit_actor, (float)BaseDamage, GetController(), this, UDamageType::StaticClass());
			}
		} else if (hit_actor->ActorHasTag(TAG_INTERACTABLE) || hit_actor->Implements<UInteractableInterface>()) {
			if (hit_actor->Implements<UInteractableInterface>()) {
				IInteractableInterface::Execute_Interact(hit_actor);
			} else {
				// Old fallback - interfaces are preferred
				UFunction* InteractFunction = hit_actor->FindFunction(FName("Interact"));
				if (InteractFunction) {
					hit_actor->ProcessEvent(InteractFunction, nullptr);
				}
			}
		}
	}
}
