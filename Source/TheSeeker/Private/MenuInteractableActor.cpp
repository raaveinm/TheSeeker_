// Created by Kirill "Raaveinm"


#include "MenuInteractableActor.h"
#include "Components/StaticMeshComponent.h"

AMenuInteractableActor::AMenuInteractableActor() {
	PrimaryActorTick.bCanEverTick = false;
	
	// Set up listeners for collision
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->OnClicked.AddDynamic(this, &AMenuInteractableActor::OnMeshClicked);
	MeshComp->OnBeginCursorOver.AddDynamic(this, &AMenuInteractableActor::OnMeshBeginCursorOver);
	MeshComp->OnEndCursorOver.AddDynamic(this, &AMenuInteractableActor::OnMeshEndCursorOver);
}

void AMenuInteractableActor::BeginPlay() { Super::BeginPlay(); }

void AMenuInteractableActor::OnMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed) {
	if (ButtonPressed == EKeys::LeftMouseButton) {
		BP_PerformAction();
	}
}

void AMenuInteractableActor::OnMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent) {
	BP_OnHoverEnter();
}

void AMenuInteractableActor::OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent) {
	BP_OnHoverExit();
}

// Called every frame
void AMenuInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

