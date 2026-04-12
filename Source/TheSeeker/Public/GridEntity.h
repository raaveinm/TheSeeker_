//
// Created by raaveinm on 4/9/26.
//
#pragma once

#include "CoreMinimal.h"
#include "ShipGridManager.h"
#include "GameFramework/Actor.h"
#include "GridEntity.generated.h"

/**
 * Base class for any object that exists on the Ship Grid (Crates, Consoles, etc.)
 */
UCLASS(Abstract, Blueprintable)
class THESEEKER_API AGridEntity : public AActor
{
	GENERATED_BODY()
	
public:	
	AGridEntity();

	// The coordinates this entity occupies on the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = "true"))
	FIntVector GridCoordinates;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetGridCoordinates(FIntVector NewCoords);

	/////////////////////////////////////////////
	// Grid Logic
	/////////////////////////////////////////////

	// Handles snapping to the grid when moved in the editor or spawned
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	// Specifically handle manual property changes in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// Handles the end of a viewport drag/move operation
	virtual void PostEditMove(bool bFinished) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SnapToGrid();

	// Returns data for all tiles that act as triggers for this entity
	UFUNCTION(BlueprintPure, Category = "Grid")
	TArray<struct FTileData> GetInteractionTileData() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	ETileState EntityGridState = ETileState::Object;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FIntVector> OccupiedOffsets = { FIntVector(0,0,0) };

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteracted(AActor* InteractingUnit);
	virtual void OnInteracted_Implementation(AActor* InteractingUnit);

protected:
	virtual void BeginPlay() override;
};
