// .


#include "MainMenuController.h"

AMainMenuController::AMainMenuController() {
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AMainMenuController::BeginPlay() {
	Super::BeginPlay();
	FInputModeGameAndUI input;
	input.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	input.SetHideCursorDuringCapture(false);
	SetInputMode(input);
}
