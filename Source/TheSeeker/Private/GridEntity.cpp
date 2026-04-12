#include "GridEntity.h"
#include "ShipGridManager.h"
#include "LevelControl.h"
#include "Kismet/GameplayStatics.h"

AGridEntity::AGridEntity()
{
	PrimaryActorTick.bCanEverTick = false;
	GridCoordinates = FIntVector::ZeroValue;
}

void AGridEntity::BeginPlay()
{
	Super::BeginPlay();

	// Auto-register with level control on start
	ALevelControl* LC = ALevelControl::FindLevelControl(this);
	if (LC) LC->RegisterMultiFloorActor(this);
	
	UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
	if (GridManager) {
		GridManager->RegisterPlacedObject(this, EntityGridState);
	}
}

void AGridEntity::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
#if WITH_EDITOR
	if (!GIsTransacting)
		SnapToGrid();
#else
	SnapToGrid();
#endif
}

#if WITH_EDITOR
void AGridEntity::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AGridEntity, GridCoordinates)) {
		UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
		float TileSize = GridManager ? GridManager->TILE_SIZE : 100.f;
		float FloorHeight = GridManager ? GridManager->FLOOR_HEIGHT : 300.f;

		FVector NewLoc = FVector(GridCoordinates.X * TileSize, GridCoordinates.Y * TileSize, GridCoordinates.Z * FloorHeight);
		SetActorLocation(NewLoc);
		
		ALevelControl::ApplyFloorTags(this, GridCoordinates.Z);
		
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
}

void AGridEntity::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished)
	{
		SnapToGrid();
	}
}
#endif

void AGridEntity::SetGridCoordinates(FIntVector NewCoords)
{
	GridCoordinates = NewCoords;
	
	UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
	float TileSize = GridManager ? GridManager->TILE_SIZE : 100.f;
	float FloorHeight = GridManager ? GridManager->FLOOR_HEIGHT : 300.f;

	FVector NewLoc = FVector(GridCoordinates.X * TileSize, GridCoordinates.Y * TileSize, GridCoordinates.Z * FloorHeight);
	SetActorLocation(NewLoc);

	ALevelControl::ApplyFloorTags(this, GridCoordinates.Z);
}

void AGridEntity::SnapToGrid()
{
	UShipGridManager* GridManager = UShipGridManager::FindGridManager(this);
	
	float TileSize = GridManager ? GridManager->TILE_SIZE : 100.f;
	float FloorHeight = GridManager ? GridManager->FLOOR_HEIGHT : 300.f;

	FVector CurrentLoc = GetActorLocation();

	float SnappedX = FMath::RoundToFloat(CurrentLoc.X / TileSize) * TileSize;
	float SnappedY = FMath::RoundToFloat(CurrentLoc.Y / TileSize) * TileSize;
	float SnappedZ = FMath::RoundToFloat(CurrentLoc.Z / FloorHeight) * FloorHeight;

	FVector SnappedLoc(SnappedX, SnappedY, SnappedZ);

	if (!CurrentLoc.Equals(SnappedLoc, 0.1f))
	{
		SetActorLocation(SnappedLoc);
	}

	GridCoordinates.X = FMath::RoundToInt(SnappedX / TileSize);
	GridCoordinates.Y = FMath::RoundToInt(SnappedY / TileSize);
	GridCoordinates.Z = FMath::RoundToInt(SnappedZ / FloorHeight);

	ALevelControl::ApplyFloorTags(this, GridCoordinates.Z);

	ALevelControl* LC = ALevelControl::FindLevelControl(this);
	if (LC) { LC->RegisterMultiFloorActor(this); }
}

TArray<FTileData> AGridEntity::GetInteractionTileData() const
{
	TArray<FTileData> TileData;
	
	UShipGridManager* GridManager = UShipGridManager::FindGridManager(const_cast<AGridEntity*>(this));
	if (!GridManager) return TileData;

	for (int32 dy = -1; dy <= 1; ++dy)
	{
		for (int32 dx = -1; dx <= 1; ++dx)
		{
			if (dx == 0 && dy == 0) continue;

			FIntVector NeighborCoords(GridCoordinates.X + dx, GridCoordinates.Y + dy, GridCoordinates.Z);
			TileData.Add(GridManager->GetTileDataAt(NeighborCoords));
		}
	}

	return TileData;
}

void AGridEntity::OnInteracted_Implementation(AActor* InteractingUnit)
{
	UE_LOG(LogTemp, Log, TEXT("AGridEntity: Interacted with by %s"), InteractingUnit ? *InteractingUnit->GetName() : TEXT("NULL"));
}
