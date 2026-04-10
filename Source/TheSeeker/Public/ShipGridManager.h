//
// Created by raaveinm on 4/8/26.
//

#pragma once

#include "CoreMinimal.h"
#include "FloorBoard.h"
#include "Components/SceneComponent.h"
#include "ShipGridManager.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THESEEKER_API UShipGridManager : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShipGridManager();
	
	/////////////////////////////////////////////
	/// Grid Setup
	/////////////////////////////////////////////
	UPROPERTY(EditAnywhere, Category="GridSetup")
	TSubclassOf<AFloorBoard> TileClass;
	
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 GRID_WIDTH = 20; // Corrected default to a reasonable tile count
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 GRID_HEIGHT = 10; // Corrected default to a reasonable tile count
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 FLOORS_AMOUNT = 3;
	UPROPERTY(EditAnywhere, Category="GridSetup")
	float TILE_SIZE = 100.f; // Standard tile size is usually 100 or 200
	UPROPERTY(EditAnywhere, Category="GridSetup")
	float FLOOR_HEIGHT = 300.f; // Height between floors
	
	/////////////////////////////////////////////
	/// Grid Manager
	/////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void BuildShip();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void ClearGrid();
	
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool TryPlaceObject(FIntVector Coords, TSubclassOf<AActor> ObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool RemoveObjectAt(FIntVector Coords);

	UFUNCTION(BlueprintPure, Category="Grid")
	AFloorBoard* GetTileWithCoordinates(FIntVector coordinates) const;

	// Static helper to find the grid manager in any level
	UFUNCTION(BlueprintPure, Category = "Grid", meta = (WorldContext = "WorldContextObject"))
	static UShipGridManager* FindGridManager(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Grid")
	bool RegisterPlacedObject(AActor* placedObject, ETileState state);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	TMap<FIntVector,AFloorBoard*> TileMap;

	// Internal helper to update neighbors when an object/doorway is placed/removed
	void SetupAdjacentTiles(FIntVector Origin, bool bIsAdding) const;
};
