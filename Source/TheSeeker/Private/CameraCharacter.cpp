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
#include "ShipGridManager.h"
#include "GridEntity.h"
#include "InteractableInterface.h"

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
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
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
	CameraBoom->CameraLagSpeed = 3.f;
	
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
	
	TargetZ = GetActorLocation().Z;
	
	AActor* actorManager = UGameplayStatics::GetActorOfClass(GetWorld(), ALevelControl::StaticClass());
	LevelManager = Cast<ALevelControl>(actorManager);
	
	// Link Master - Slave
	if (!controlledUnit) {
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

	FVector NewLocation = GetActorLocation() + 
		(forwardDirection * movementVector.Y + rightDirection * movementVector.X)* 
			GetCharacterMovement()->MaxFlySpeed * GetWorld()->GetDeltaSeconds();
	SetActorLocation(NewLocation, true);
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
		if (controlledUnit) {
			Params.AddIgnoredActor(controlledUnit);
		}

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, COLLISION_FLOOR, Params)) {

			UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
			float RelativeZ = Hit.Location.Z;

			if (GridManager) {
				RelativeZ = GridManager->GetComponentTransform().InverseTransformPosition(Hit.Location).Z;
			}

			int32 CalculatedFloor = FMath::Clamp(
				FMath::FloorToInt((RelativeZ + 10.f) / LevelManager->FLOOR_HEIGHT),
				LevelManager->MIN_FLOOR,
				LevelManager->MAX_FLOOR
			);

			bool bInManualCooldown = GetWorld()->GetTimeSeconds() - LastManualFloorTime <= AutoFloorCooldown;
			if (!bInManualCooldown || CalculatedFloor == LevelManager->CURRENT_FLOOR) {
				TargetZ = Hit.Location.Z + 100.f;

				if (!bInManualCooldown) {
					LevelManager->SetFloor(CalculatedFloor);
				}
			}
		}
	}

	if (FMath::Abs(GetActorLocation().Z - TargetZ) > 0.1f)
	{
		FVector NewLoc = GetActorLocation();
		NewLoc.Z = FMath::FInterpTo(NewLoc.Z, TargetZ, DeltaTime, HeightInterpSpeed);
		SetActorLocation(NewLoc);
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
			enhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ACameraCharacter::InteractInput);
	}
}

///////////////////////////////////////////////
/// Levels Control
///////////////////////////////////////////////

void ACameraCharacter::InteractInput(const FInputActionValue& val) {
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
	if (!GridManager) return;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
		FIntVector Coords = GridManager->WorldToGrid(Hit.Location);
		OnInteract(val, Coords);
	} else {
		OnInteract(val, FIntVector(-1, -1, -1));
	}
}

void ACameraCharacter::OnInteract_Implementation(const FInputActionValue& val, FIntVector TargetCoords) {
	if (!controlledUnit) return;

	UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
	if (!GridManager) return;

	FIntVector UnitCoords = GridManager->WorldToGrid(controlledUnit->GetActorLocation());
	FTileData UnitTile = GridManager->GetTileDataAt(UnitCoords);

	if (UnitTile.State != ETileState::Trigger) {
		UE_LOG(LogTemp, Warning, TEXT("ACameraCharacter: Unit is not on a Trigger tile."));
		return;
	}

	FTileData TargetTile = GridManager->GetTileDataAt(TargetCoords);
	if (TargetTile.State == ETileState::Object || TargetTile.State == ETileState::Doorway) {
		if (UnitTile.ParentObjectCoordinates.Contains(TargetCoords)) {
			AActor* ObjectToInteract = TargetTile.OccupyingActor;
			if (ObjectToInteract) {
				if (ObjectToInteract->Implements<UInteractableInterface>()) {
					IInteractableInterface::Execute_Interact(ObjectToInteract);
				} 
				else if (AGridEntity* GridEntity = Cast<AGridEntity>(ObjectToInteract)) {
					GridEntity->OnInteracted(controlledUnit);
				}
			}
		} else {
			UE_LOG(LogTemp, Log, TEXT("ACameraCharacter: Clicked object is not a parent of the current trigger tile."));
		}
	}
}

void ACameraCharacter::ChangeFloorUp(const FInputActionValue& val) {
	if (LevelManager && LevelManager->CURRENT_FLOOR < LevelManager->MAX_FLOOR) {
		LastManualFloorTime = GetWorld()->GetTimeSeconds();
		LevelManager->FloorUp();
		
		TargetZ = (LevelManager->CURRENT_FLOOR * LevelManager->FLOOR_HEIGHT) + 100.f;
	}
}

void ACameraCharacter::ChangeFloorDown(const FInputActionValue& val) {
	if (LevelManager && LevelManager->CURRENT_FLOOR > LevelManager->MIN_FLOOR) { 
		LastManualFloorTime = GetWorld()->GetTimeSeconds();
		LevelManager->FloorDown();

		TargetZ = (LevelManager->CURRENT_FLOOR * LevelManager->FLOOR_HEIGHT) + 100.f;
	}
}
