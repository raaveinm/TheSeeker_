//
// Created by raaveinm on 4/8/26.
//

#include "ShipGridManager.h"
#include <ranges>

// Sets default values for this component's properties
UShipGridManager::UShipGridManager() {
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UShipGridManager::BeginPlay() {
	Super::BeginPlay();
}

void UShipGridManager::ClearGrid() {
	for (auto& Elem : TileMap) {
		if (Elem.Value) {
			Elem.Value->Destroy();
		}
	}
	TileMap.Empty();
}

void UShipGridManager::BuildShip() {
	if (TileClass == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("UShipGridManager: TileClass is NULL!"));
		return;
	}

	ClearGrid();
	
	for (int32 z = 0; z < FLOORS_AMOUNT; ++z) {
		for (int32 y = 0; y < GRID_HEIGHT; ++y) {
			for (int32 x = 0; x < GRID_WIDTH; ++x) {
				FIntVector coordinate(x, y, z);
				FVector location = FVector(x * TILE_SIZE, y * TILE_SIZE, z * FLOOR_HEIGHT);
				
				FActorSpawnParameters spawnParams;
				spawnParams.Owner = GetOwner();
				spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AFloorBoard* tile = GetWorld()->SpawnActor<AFloorBoard>(TileClass, spawnParams);

				if (tile) {
					tile->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
					tile->SetActorRelativeLocation(location);
					tile->GridCoordinates = coordinate;
                    
					// Tagging for floor visibility logic
					tile->Tags.Add(FName("MultiFloor"));
					tile->Tags.Add(FName(*FString::Printf(TEXT("Floor_%d"), z)));
                    
					TileMap.Add(coordinate, tile);
				}
			}
		}
	}
}

bool UShipGridManager::TryPlaceObject(FIntVector Coords, TSubclassOf<AActor> ObjectClass) {
	AFloorBoard* targetTile = GetTileWithCoordinates(Coords);
	if (!targetTile || !ObjectClass) return false;

	if (targetTile->TileData.State != ETileState::Floor && targetTile->TileData.State != ETileState::Trigger)
		return false;

	// Spawn the actual object actor
	FVector spawnLoc = targetTile->GetActorLocation();
	AActor* placedActor = GetWorld()->SpawnActor<AActor>(ObjectClass, spawnLoc, FRotator::ZeroRotator);
	
	if (placedActor) {
		targetTile->TileData.OccupyingActor = placedActor;
		targetTile->SetTileState(ETileState::Object);
		
		// Update neighbors to create triggers
		SetupAdjacentTiles(Coords, true);
		return true;
	}

	return false;
}

bool UShipGridManager::RemoveObjectAt(FIntVector Coords) {
	AFloorBoard* targetTile = GetTileWithCoordinates(Coords);
	if (!targetTile || targetTile->TileData.State != ETileState::Object) return false;

	if (targetTile->TileData.OccupyingActor) {
		targetTile->TileData.OccupyingActor->Destroy();
		targetTile->TileData.OccupyingActor = nullptr;
	}

	targetTile->SetTileState(ETileState::Floor);
	
	// Notify neighbors to remove this parent coordinate
	SetupAdjacentTiles(Coords, false);
	
	return true;
}

void UShipGridManager::SetupAdjacentTiles(FIntVector Origin, bool bIsAdding) const {
	// Standard 8-neighbor adjacency on the same floor
	for (int32 dy : std::views::iota(-1, 2)) {
		for (int32 dx : std::views::iota(-1, 2)) {
			// Skip the origin tile itself
			if (dx == 0 && dy == 0) continue;

			FIntVector neighborCoords(Origin.X + dx, Origin.Y + dy, Origin.Z);
			AFloorBoard* neighborTile = GetTileWithCoordinates(neighborCoords);
			
			if (neighborTile) {
				if (bIsAdding) {
					neighborTile->AddParentObjectCoordinate(Origin);
				} else {
					neighborTile->RemoveParentObjectCoordinate(Origin);
				}
			}
		}
	}
}

AFloorBoard* UShipGridManager::GetTileWithCoordinates(FIntVector coordinates) const {
	return TileMap.FindRef(coordinates);
}

UShipGridManager* UShipGridManager::FindGridManager(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	for (TObjectIterator<UShipGridManager> It; It; ++It)
	{
		if (It->GetWorld() == World)
		{
			return *It;
		}
	}
	return nullptr;
}

bool UShipGridManager::RegisterPlacedObject(AActor* placedObject, ETileState state) {
	if (!placedObject) return false;
	
	FVector spawnLoc = placedObject->GetActorLocation();
	FVector coordinate(
		FMath::RoundToInt(spawnLoc.X / TILE_SIZE),
		FMath::RoundToInt(spawnLoc.Y / TILE_SIZE),
		FMath::RoundToInt(spawnLoc.Z / FLOOR_HEIGHT)
	);
	
	AFloorBoard* target = GetTileWithCoordinates(FIntVector(coordinate));
	if (!target) return false;
	
	target->TileData.OccupyingActor = placedObject;
	target->SetTileState(state);

	if (state == ETileState::Object || state == ETileState::Doorway) {
		SetupAdjacentTiles(static_cast<FIntVector>(coordinate), true);
	}
	
	return true;
}

void UShipGridManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
