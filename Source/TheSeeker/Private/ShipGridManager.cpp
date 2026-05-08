//
// Created by raaveinm on 4/8/26.
//

#include "ShipGridManager.h"
#include "LevelControl.h"
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
	TileDataMap.Empty();

	// Clear ISMC instances and the map
	for (auto& Pair : StateISMCMap) {
		if (Pair.Value) {
			Pair.Value->DestroyComponent();
		}
	}
	StateISMCMap.Empty();

	// Destroy floor container actors
	for (auto& Pair : FloorContainers) {
		if (Pair.Value) {
			Pair.Value->Destroy();
		}
	}
	FloorContainers.Empty();
	
	TileVisualsMap.Empty();
}

UInstancedStaticMeshComponent* UShipGridManager::GetOrCreateISMC(int32 Floor, ETileState State) {
	FString Key = FString::Printf(TEXT("%d_%d"), Floor, (int32)State);
	
	if (UInstancedStaticMeshComponent** FoundISMC = StateISMCMap.Find(Key)) {
		if (*FoundISMC) return *FoundISMC;
	}

	if (!FloorContainers.Contains(Floor)) {
		AActor* Container = GetWorld()->SpawnActor<AActor>(AActor::StaticClass());
		Container->SetActorLabel(FString::Printf(TEXT("FloorContainer_%d"), Floor));
		
		USceneComponent* Root = NewObject<USceneComponent>(Container, TEXT("Root"));
		Container->SetRootComponent(Root);
		Root->RegisterComponent();
		Container->AttachToActor(GetOwner(), FAttachmentTransformRules::KeepRelativeTransform);
		ALevelControl::ApplyFloorTags(Container, Floor);
		
		if (ALevelControl* LC = ALevelControl::FindLevelControl(this)) {
			LC->RegisterMultiFloorActor(Container);
		}
		
		FloorContainers.Add(Floor, Container);
	}

	AActor* ParentActor = FloorContainers[Floor];
	FName CompName = *FString::Printf(TEXT("ISMC_F%d_S%d"), Floor, (int32)State);
	
	UInstancedStaticMeshComponent* NewISMC = NewObject<UInstancedStaticMeshComponent>(ParentActor, CompName);
	NewISMC->RegisterComponent();
	NewISMC->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	
	if (StateMeshMap.Contains(State)) {
		NewISMC->SetStaticMesh(StateMeshMap[State]);
	}

	NewISMC->SetCollisionProfileName(TEXT("BlockAll"));
	NewISMC->ComponentTags.Add(ALevelControl::TAG_MULTI_FLOOR);
	NewISMC->ComponentTags.Add(FName(*FString::Printf(TEXT("Floor_%d"), Floor)));

	StateISMCMap.Add(Key, NewISMC);
	return NewISMC;
}

void UShipGridManager::SetTileState(FIntVector Coords, ETileState NewState) {
	if (FTileData* Data = TileDataMap.Find(Coords)) {
		if (Data->State != NewState) {
			Data->State = NewState;
			UpdateTileVisuals(Coords);
		}
	}
}

void UShipGridManager::RefreshTileState(FIntVector Coords) {
	FTileData* Data = TileDataMap.Find(Coords);
	if (!Data) return;

	if (Data->State != ETileState::Floor && Data->State != ETileState::Trigger) {
		return;
	}

	if (Data->ParentObjectCoordinates.Num() > 0) {
		SetTileState(Coords, ETileState::Trigger);
	} else {
		SetTileState(Coords, ETileState::Floor);
	}
}

void UShipGridManager::UpdateTileVisuals(FIntVector Coords) {
	FTileData* Data = TileDataMap.Find(Coords);
	if (!Data) return;

	RemoveInstanceForTile(Coords);

	UInstancedStaticMeshComponent* TargetISMC = GetOrCreateISMC(Coords.Z, Data->State);
	if (TargetISMC) {
		FTransform Transform;
		Transform.SetLocation(GridToWorld(Coords));
		
		int32 NewIndex = TargetISMC->AddInstance(Transform, true);
		
		FTileVisualInstance Visual;
		Visual.ISMC = TargetISMC;
		Visual.InstanceIndex = NewIndex;
		TileVisualsMap.Add(Coords, Visual);
	}
}

void UShipGridManager::RemoveInstanceForTile(FIntVector Coords) {
	if (TileVisualsMap.Contains(Coords)) {
		FTileVisualInstance Visual = TileVisualsMap[Coords];
		if (Visual.ISMC && Visual.InstanceIndex != -1) {
			Visual.ISMC->RemoveInstance(Visual.InstanceIndex);
			
			// Shift indices for other tiles using the SAME ISMC
			for (auto& Pair : TileVisualsMap) {
				if (Pair.Value.ISMC == Visual.ISMC && Pair.Value.InstanceIndex > Visual.InstanceIndex) {
					Pair.Value.InstanceIndex--;
				}
			}
		}
		TileVisualsMap.Remove(Coords);
	}
}

void UShipGridManager::BuildShip() {
	ClearGrid();
	
	for (int32 z = 0; z < FLOORS_AMOUNT; ++z) {
		for (int32 y = 0; y < GRID_HEIGHT; ++y) {
			for (int32 x = 0; x < GRID_WIDTH; ++x) {
				FIntVector coordinate(x, y, z);
				
				FTileData NewData;
				NewData.State = ETileState::Floor;
				TileDataMap.Add(coordinate, NewData);

				UpdateTileVisuals(coordinate);
			}
		}
	}

	if (ALevelControl* LC = ALevelControl::FindLevelControl(this)) {
		LC->UpdateFloorVisibility();
	}
}

bool UShipGridManager::TryPlaceObject(FIntVector Coords, TSubclassOf<AActor> ObjectClass) {
	FTileData* Data = TileDataMap.Find(Coords);
	if (!Data || !ObjectClass) return false;

	if (Data->State != ETileState::Floor && Data->State != ETileState::Trigger)
		return false;

	FVector spawnLoc = GridToWorld(Coords);
	AActor* placedActor = GetWorld()->SpawnActor<AActor>(ObjectClass, spawnLoc, FRotator::ZeroRotator);
	
	if (placedActor) {
		Data->OccupyingActor = placedActor;

		// Register with Level Control
		ALevelControl::ApplyFloorTags(placedActor, Coords.Z);
		if (ALevelControl* LC = ALevelControl::FindLevelControl(this)) {
			LC->RegisterMultiFloorActor(placedActor);
		}

		SetTileState(Coords, ETileState::Object);
		
		SetupAdjacentTiles(Coords, true);
		return true;
	}

	return false;
}

bool UShipGridManager::RemoveObjectAt(FIntVector Coords) {
	FTileData* Data = TileDataMap.Find(Coords);
	if (!Data || Data->State != ETileState::Object) return false;

	if (Data->OccupyingActor) {
		Data->OccupyingActor->Destroy();
		Data->OccupyingActor = nullptr;
	}

	SetTileState(Coords, ETileState::Floor);
	RefreshTileState(Coords);
	
	SetupAdjacentTiles(Coords, false);
	
	return true;
}

void UShipGridManager::SetupAdjacentTiles(FIntVector Origin, bool bIsAdding) {
	for (int32 dy : std::views::iota(-1, 2)) {
		for (int32 dx : std::views::iota(-1, 2)) {
			if (dx == 0 && dy == 0) continue;

			FIntVector neighborCoords(Origin.X + dx, Origin.Y + dy, Origin.Z);
			if (bIsAdding) {
				AddParentObjectToTile(neighborCoords, Origin);
			} else {
				RemoveParentObjectFromTile(neighborCoords, Origin);
			}
		}
	}
}

FTileData UShipGridManager::GetTileDataAt(FIntVector Coords) const {
	if (const FTileData* Data = TileDataMap.Find(Coords)) {
		return *Data;
	}
	return FTileData();
}

FIntVector UShipGridManager::WorldToGrid(FVector WorldLocation) const {
	FVector RelativeLoc = GetComponentTransform().InverseTransformPosition(WorldLocation);
	return FIntVector(
		FMath::RoundToInt(RelativeLoc.X / TILE_SIZE),
		FMath::RoundToInt(RelativeLoc.Y / TILE_SIZE),
		FMath::RoundToInt(RelativeLoc.Z / FLOOR_HEIGHT)
	);
}

FVector UShipGridManager::GridToWorld(FIntVector Coords) const {
	FVector RelativeLoc(Coords.X * TILE_SIZE, Coords.Y * TILE_SIZE, Coords.Z * FLOOR_HEIGHT);
	return GetComponentTransform().TransformPosition(RelativeLoc);
}

UShipGridManager* UShipGridManager::FindGridManager(UObject* WorldContextObject) {
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	for (TObjectIterator<UShipGridManager> It; It; ++It) {
		if (It->GetWorld() == World) return *It;
	}
	return nullptr;
}

bool UShipGridManager::RegisterPlacedObject(AActor* placedObject, ETileState state) {
	if (!placedObject) return false;
	
	FIntVector coordinate = WorldToGrid(placedObject->GetActorLocation());
	FTileData* Data = TileDataMap.Find(coordinate);
	if (!Data) return false;
	
	Data->OccupyingActor = placedObject;

	ALevelControl::ApplyFloorTags(placedObject, coordinate.Z);
	if (ALevelControl* LC = ALevelControl::FindLevelControl(this)) {
		LC->RegisterMultiFloorActor(placedObject);
	}

	SetTileState(coordinate, state);

	if (state == ETileState::Object || state == ETileState::Doorway) {
		SetupAdjacentTiles(coordinate, true);
	}
	
	return true;
}

void UShipGridManager::AddParentObjectToTile(FIntVector TileCoords, FIntVector ParentCoords) {
	if (FTileData* Data = TileDataMap.Find(TileCoords)) {
		if (!Data->ParentObjectCoordinates.Contains(ParentCoords)) {
			Data->ParentObjectCoordinates.Add(ParentCoords);
			if (Data->State == ETileState::Floor) {
				SetTileState(TileCoords, ETileState::Trigger);
			}
		}
	}
}

void UShipGridManager::RemoveParentObjectFromTile(FIntVector TileCoords, FIntVector ParentCoords) {
	if (FTileData* Data = TileDataMap.Find(TileCoords)) {
		if (Data->ParentObjectCoordinates.Contains(ParentCoords)) {
			Data->ParentObjectCoordinates.Remove(ParentCoords);
			if (Data->State == ETileState::Trigger && Data->ParentObjectCoordinates.IsEmpty()) {
				SetTileState(TileCoords, ETileState::Floor);
			}
		}
	}
}

void UShipGridManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

