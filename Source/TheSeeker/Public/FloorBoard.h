//
// Created by raaveinm on 4/8/26.
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorBoard.generated.h"

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

UCLASS()
class THESEEKER_API AFloorBoard : public AActor
{
    GENERATED_BODY()
    
public: 
    AFloorBoard();

    virtual void Tick(float DeltaTime) override;

    /////////////////////////////////////////////
    // Tile Data & State
    /////////////////////////////////////////////

    // The data representing this specific tile's state and contents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Board")
    FTileData TileData;

    // The grid coordinates of this tile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Board")
    FIntVector GridCoordinates;

    /////////////////////////////////////////////
    // Components
    /////////////////////////////////////////////

    // Visual representation of the tile
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TileMesh;

    /////////////////////////////////////////////
    // Tile Logic (Blueprint & C++ Symbiote)
    /////////////////////////////////////////////

    // Updates the tile state and handles visual/logic changes
    UFUNCTION(BlueprintCallable, Category = "Floor Board")
    virtual void SetTileState(ETileState NewState);

    // Blueprint implementable event to handle visual changes when state updates (e.g., changing material or visibility)
    UFUNCTION(BlueprintImplementableEvent, Category = "Floor Board")
    void OnTileStateChanged(ETileState NewState);

    // Adds a parent object coordinate (for overlapping triggers)
    UFUNCTION(BlueprintCallable, Category = "Floor Board")
    void AddParentObjectCoordinate(FIntVector Coordinate);

    // Removes a parent object coordinate and updates state if needed
    UFUNCTION(BlueprintCallable, Category = "Floor Board")
    void RemoveParentObjectCoordinate(FIntVector Coordinate);

    // Checks if the tile has any active triggers
    UFUNCTION(BlueprintPure, Category = "Floor Board")
    bool HasActiveTriggers() const;
    
protected:
    virtual void BeginPlay() override;
};
