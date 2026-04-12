// Created by Kirill "Raaveinm"


#include "StaticCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"

// Sets default values
AStaticCameraPawn::AStaticCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AStaticCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	Rotation = CameraComp->GetRelativeRotation();
}

// Called every frame
void AStaticCameraPawn::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	APlayerController* controller = Cast<APlayerController>(GetController());
	if (!controller) return;
	
	if (!CameraFollowEnabled) return;
	
	float cursor_x, cursor_y;	
	if (controller->GetMousePosition(cursor_x, cursor_y)) {
		int32 viewport_size_x, viewport_size_y;
		controller->GetViewportSize(viewport_size_x, viewport_size_y);

		if (viewport_size_x > 0 && viewport_size_y > 0) {
			float normal_x = cursor_x / static_cast<float>(viewport_size_x) * 2.f - 1.f;
			float normal_y = cursor_y / static_cast<float>(viewport_size_y) * 2.f - 1.f;
			
			FRotator target = Rotation + FRotator(normal_y * -MaxPitchDeflection, normal_x * MaxYawDeflection, 0.f);
			FRotator target_rotation = FMath::RInterpTo(CameraComp->GetRelativeRotation(), target, DeltaTime, InterpSpeed);
			CameraComp->SetRelativeRotation(target_rotation);
		}
	}
}

// Called to bind functionality to input
void AStaticCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
