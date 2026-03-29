// Created by Kirill "Raaveinm"

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "MenuInteractableActor.generated.h"

class UStaticMeshComponent; 
class UPrimitiveComponent;

UCLASS()
class THESEEKER_API AMenuInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMenuInteractableActor();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	/////////////////////////////////////////////
	/// Mouse Interaction
	/////////////////////////////////////////////
	UFUNCTION()
	void OnMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void OnMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnMeshEndCursorOver(UPrimitiveComponent* TouchedComponent);

	/////////////////////////////////////////////
	/// Menu Blueprint Interface
	/////////////////////////////////////////////
	UFUNCTION(BlueprintImplementableEvent, Category = "Menu Logic")
	void BP_PerformAction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu Logic")
	void BP_OnHoverEnter();

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu Logic")
	void BP_OnHoverExit();
};
