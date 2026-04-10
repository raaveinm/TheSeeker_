//
// Created by raaveinm on 2/17/26.
//

#include "CameraCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "LevelControl.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UnitCharacter.h"
#include "TheSeeker.h"
#include "FloorBoard.h"

const FName ACameraCharacter::TAG_RTS_CAMERA(TEXT("RTS_Camera"));

///////////////////////////////////////////////
/// Default Movement
///////////////////////////////////////////////
ACameraCharacter::ACameraCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// detaching camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// character setup
    GetCapsuleComponent()->SetCollisionProfileName(TAG_RTS_CAMERA);
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying; 
	GetCharacterMovement()->MaxFlySpeed = 1200.f; 
	GetCharacterMovement()->BrakingDecelerationFlying = 2000.f;

    // camera setup
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = LENGTH;
	CameraBoom->SetRelativeRotation(FRotator(PITCH, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	// CameraBoom->CameraLagSpeed = 3.f;
	
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;
	TopDownCamera->FieldOfView = FOV;

	// 3rd person character doesn't rotate with controller, but rotates with movement direction
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ACameraCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	
	APlayerController* playerController = Cast<APlayerController>(NewController);
	
	if (!playerController) return;
	
	playerController->bShowMouseCursor = true;
	playerController->bEnableClickEvents = true;
	playerController->bEnableMouseOverEvents = true;
	
	UEnhancedInputLocalPlayerSubsystem* sys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer());
	if (!sys) return;
	
	if (DefaultMappingContext) {
		sys->AddMappingContext(DefaultMappingContext, 0);
	}
}

///////////////////////////////////////////////
/// Begin Game
///////////////////////////////////////////////
void ACameraCharacter::BeginPlay() {
	Super::BeginPlay();
	
	AActor* actorManager = UGameplayStatics::GetActorOfClass(GetWorld(), ALevelControl::StaticClass());
	LevelManager = Cast<ALevelControl>(actorManager);
	
	// Link Master - Slave
	if (!controlledUnit) 
	{
		controlledUnit = Cast<AUnitCharacter>(GetOwner());
	}

	if (!controlledUnit) 
	{
		UE_LOG(LogTemp, Warning, TEXT("ACameraCharacter: No UnitCharacter assigned via Editor or Owner!"));
	}

	// Get the player controller and add input mapping context
	if (APlayerController* playerController = Cast<APlayerController>(Controller)) {
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
		playerController->bEnableMouseOverEvents = true;

		if (UEnhancedInputLocalPlayerSubsystem* subsys = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer())) {
			if (DefaultMappingContext) {
				subsys->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

///////////////////////////////////////////////
/// Movement handling
///////////////////////////////////////////////

void ACameraCharacter::Command(const FInputActionValue& val) {
	if (!controlledUnit) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No Unit Controlled!"));
		return;
	}
	
	APlayerController* playerController = Cast<APlayerController>(Controller);
	if (!playerController) return;
	
	FHitResult hit;
	// Ensure we only command movement to valid locations
	if (playerController->GetHitResultUnderCursor(ECC_Visibility, false, hit)) {
		if (hit.GetActor()) {
			DrawDebugSphere(GetWorld(), hit.Location, 50.f, 12, FColor::Magenta, false, 1.f);
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(controlledUnit->GetController(), hit.Location);
		}
	}
}

void ACameraCharacter::Move(const FInputActionValue& val) {
	if (Controller == nullptr) return;

	FVector2D movementVector = val.Get<FVector2D>();

	const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0, rotation.Yaw, 0);
	const FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(forwardDirection, movementVector.Y);
	AddMovementInput(rightDirection, movementVector.X);
}


void ACameraCharacter::Zoom(const FInputActionValue& val) {
	if (!CameraBoom) return;
	float zoom = val.Get<float>();
	float new_length = CameraBoom->TargetArmLength - (zoom * SPEED);
	CameraBoom->TargetArmLength = FMath::Clamp(new_length, MIN_ZOOM, MAX_ZOOM);
}


///////////////////////////////////////////////
/// Tick
///////////////////////////////////////////////
void ACameraCharacter::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);
	
	if (!LevelManager) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastTraceTime >= TraceFrequency) {
		LastTraceTime = CurrentTime;

		FHitResult Hit;
		FVector Start = GetActorLocation();
		Start.Z += TraceDistanceAbove;
		FVector End = Start - FVector(0, 0, TraceDistanceBelow);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, COLLISION_FLOOR, Params)) {

			int32 CalculatedFloor = FMath::Clamp(
				FMath::FloorToInt((Hit.Location.Z + 10.f) / LevelManager->FLOOR_HEIGHT),
				LevelManager->MIN_FLOOR,
				LevelManager->MAX_FLOOR
			);
			LevelManager->SetFloor(CalculatedFloor);

			float DesiredHeight = 100.f;
			float TargetZ = Hit.Location.Z + DesiredHeight;

			if (FMath::Abs(GetActorLocation().Z - TargetZ) > 1.f)
			{
				FVector NewLoc = GetActorLocation();
				NewLoc.Z = FMath::FInterpTo(NewLoc.Z, TargetZ, DeltaTime, 10.f);
				SetActorLocation(NewLoc);
			}
		}
	}
}

///////////////////////////////////////////////
/// Bind Input Functionality
///////////////////////////////////////////////
void ACameraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (enhancedInput) {
		if (MoveAction)
			enhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACameraCharacter::Move);
		if (ZoomAction)
			enhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACameraCharacter::Zoom);
		if (CommandAction)
			enhancedInput->BindAction(CommandAction, ETriggerEvent::Started, this, &ACameraCharacter::Command);
		if (FloorUpAction)
			enhancedInput->BindAction(FloorUpAction, ETriggerEvent::Started, this, &ACameraCharacter::ChangeFloorUp);
		if (FloorDownAction)
			enhancedInput->BindAction(FloorDownAction, ETriggerEvent::Started, this, &ACameraCharacter::ChangeFloorDown);
		if (InteractAction)
			enhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ACameraCharacter::OnInteract, static_cast<AFloorBoard*>(nullptr));
	}
}

///////////////////////////////////////////////
/// Levels Control
///////////////////////////////////////////////

void ACameraCharacter::OnInteract_Implementation(const FInputActionValue& val, AFloorBoard* TargetTile) {
	if (TargetTile) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
		AFloorBoard* FoundTile = Cast<AFloorBoard>(Hit.GetActor());
		OnInteract(val, FoundTile);
	}
}

void ACameraCharacter::ChangeFloorUp(const FInputActionValue& val) {
	if (LevelManager && LevelManager->CURRENT_FLOOR < LevelManager->MAX_FLOOR) {
		LevelManager->FloorUp();
	}
}

void ACameraCharacter::ChangeFloorDown(const FInputActionValue& val) {
	if (LevelManager && LevelManager->CURRENT_FLOOR > LevelManager->MIN_FLOOR) { 
		LevelManager->FloorDown();
	}
}
