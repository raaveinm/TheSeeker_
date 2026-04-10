//
// Created by raaveinm on 4/8/26.
//


#include "FloorBoard.h"
#include "Components/StaticMeshComponent.h"

AFloorBoard::AFloorBoard() {
    PrimaryActorTick.bCanEverTick = false;
    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;
    
    // default collision profile
    TileMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // Initialize default state
    TileData.State = ETileState::Floor;
    GridCoordinates = FIntVector::ZeroValue;
}

void AFloorBoard::BeginPlay() {
    Super::BeginPlay();
    OnTileStateChanged(TileData.State);
}

void AFloorBoard::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AFloorBoard::SetTileState(ETileState NewState) {
    if (TileData.State != NewState) {
        TileData.State = NewState;
        // Notify Blueprint
        OnTileStateChanged(NewState);
    }
}

void AFloorBoard::AddParentObjectCoordinate(FIntVector Coordinate) {
    if (!TileData.ParentObjectCoordinates.Contains(Coordinate)) {
        TileData.ParentObjectCoordinates.Add(Coordinate);
        // If this tile is currently a floor, promote it to a Trigger
        if (TileData.State == ETileState::Floor) {
            SetTileState(ETileState::Trigger);
        }
    }
}

void AFloorBoard::RemoveParentObjectCoordinate(FIntVector Coordinate) {
    if (TileData.ParentObjectCoordinates.Contains(Coordinate)) {
        TileData.ParentObjectCoordinates.Remove(Coordinate);
        
        // If it was a trigger and no longer has any parents, revert to Floor
        if (TileData.State == ETileState::Trigger && TileData.ParentObjectCoordinates.IsEmpty()) 
            SetTileState(ETileState::Floor);
    }
}

bool AFloorBoard::HasActiveTriggers() const {
    return TileData.State == ETileState::Trigger && !TileData.ParentObjectCoordinates.IsEmpty();
}
