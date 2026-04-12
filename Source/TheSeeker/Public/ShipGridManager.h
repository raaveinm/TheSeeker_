//
// Created by raaveinm on 4/8/26.
//

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ShipGridManager.generated.h"

UENUM(BlueprintType)
enum class ETileState : uint8 {
    Beyond      UMETA(DisplayName = "Beyond"),
    Wall        UMETA(DisplayName = "Wall"),
    Floor       UMETA(DisplayName = "Floor"),
    Object      UMETA(DisplayName = "Object"),
    Trigger     UMETA(DisplayName = "Trigger"),
    Doorway     UMETA(DisplayName = "Doorway")
};

USTRUCT(BlueprintType)
struct FTileData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    ETileState State = ETileState::Floor;

    // Array to support multiple overlapping interaction zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    TArray<FIntVector> ParentObjectCoordinates; 

    // Pointer to the actual Actor (if State is Object, Doorway, or Wall)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Data")
    AActor* OccupyingActor = nullptr; 
};

USTRUCT(BlueprintType)
struct FTileVisualInstance {
	GENERATED_BODY()

	UPROPERTY()
	UInstancedStaticMeshComponent* ISMC = nullptr;

	UPROPERTY()
	int32 InstanceIndex = -1;
};

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

	// Map to define which mesh represents which state
	UPROPERTY(EditAnywhere, Category = "GridSetup")
	TMap<ETileState, UStaticMesh*> StateMeshMap;
	
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 GRID_WIDTH = 20; 
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 GRID_HEIGHT = 10; 
	UPROPERTY(EditAnywhere, Category="GridSetup")
	int32 FLOORS_AMOUNT = 3;
	UPROPERTY(EditAnywhere, Category="GridSetup")
	float TILE_SIZE = 100.f; 
	UPROPERTY(EditAnywhere, Category="GridSetup")
	float FLOOR_HEIGHT = 300.f; 
	
	/////////////////////////////////////////////
	/// Grid Manager
	/////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void BuildShip();

	// Purely resets data and ISMCs
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Grid")
	void ClearGrid();

	// Visual and Logic updates
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetTileState(FIntVector Coords, ETileState NewState);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void RefreshTileState(FIntVector Coords);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void UpdateTileVisuals(FIntVector Coords);
	
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool TryPlaceObject(FIntVector Coords, TSubclassOf<AActor> ObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool RemoveObjectAt(FIntVector Coords);

	UFUNCTION(BlueprintPure, Category="Grid")
	FTileData GetTileDataAt(FIntVector Coords) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FIntVector WorldToGrid(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GridToWorld(FIntVector Coords) const;

	// Static helper to find the grid manager in any level
	UFUNCTION(BlueprintPure, Category = "Grid", meta = (WorldContext = "WorldContextObject"))
	static UShipGridManager* FindGridManager(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Grid")
	bool RegisterPlacedObject(AActor* placedObject, ETileState state);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void AddParentObjectToTile(FIntVector TileCoords, FIntVector ParentCoords);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void RemoveParentObjectFromTile(FIntVector TileCoords, FIntVector ParentCoords);
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	
private:
	// The core data map replacing actors
	UPROPERTY()
	TMap<FIntVector, FTileData> TileDataMap;

	// Key: "FloorIndex_StateIndex"
	UPROPERTY()
	TMap<FString, UInstancedStaticMeshComponent*> StateISMCMap;

	// Maps floor index to a container actor for grouping
	UPROPERTY()
	TMap<int32, AActor*> FloorContainers;

	// Tracks which instance belongs to which tile
	UPROPERTY()
	TMap<FIntVector, FTileVisualInstance> TileVisualsMap;

	// Internal helper to update neighbors when an object/doorway is placed/removed
	void SetupAdjacentTiles(FIntVector Origin, bool bIsAdding);

	// Helper to ensure ISMCs are created and ready for a specific floor/state
	UInstancedStaticMeshComponent* GetOrCreateISMC(int32 Floor, ETileState State);
	
	void RemoveInstanceForTile(FIntVector Coords);
};
