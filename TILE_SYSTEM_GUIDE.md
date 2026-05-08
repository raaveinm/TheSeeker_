# Optimized Spaceship Tile & Interaction System Guide

This document outlines the usage and implementation of the Data-Driven Grid System for *The Seeker*. The system has been optimized to use **Instanced Static Mesh Components (ISMC)** and a **Coordinate-to-Data Map**, completely removing the overhead of per-tile Actors.

---

## 1. Tile Data & States (`ETileState`)

Tiles no longer exist as individual Actors. Instead, all tile information is stored in a `TMap<FIntVector, FTileData>` managed by `UShipGridManager`.

| State | Description | Mechanics |
| :--- | :--- | :--- |
| **Beyond** | Vacuum/Outer Space | Non-walkable, non-buildable. |
| **Wall** | Structural Boundary | Blocks movement and Line of Sight (LoS). |
| **Floor** | Walkable Terrain | Default interior state. Buildable. |
| **Object** | Occupied Tile | Contains a `GridEntity` (Console, Crate, etc.). |
| **Trigger** | Interaction Zone | A logic zone adjacent to an `Object` or `Doorway`. |
| **Doorway** | Specialized Object | Structural tile that triggers logic when a unit enters an adjacent `Trigger`. |

---

## 2. Using the System in Blueprints

### 2.1 Configuration (`UShipGridManager`)
To define the visual appearance of each state, use the **State Mesh Map** property on the `ShipGridManager` component.
*   Add an entry for each `ETileState` and assign the desired `StaticMesh`.
*   The system automatically creates and manages ISMCs for these meshes.

### 2.2 Placing Objects
1.  Get a reference to the `ShipGridManager`.
2.  Call `TryPlaceObject(FIntVector Coords, TSubclassOf<AActor> ObjectClass)`.
3.  The manager spawns the actor at the correct world location and updates the grid data and ISMCs.

### 2.3 Interaction Validation
Since tiles are no longer actors, interaction is handled via coordinates:
1.  **Raycast:** Perform a trace to get a world location.
2.  **Conversion:** Call `GridManager->WorldToGrid(HitLocation)` to get coordinates.
3.  **Validation:** Call `GridManager->GetTileDataAt(Coords)` to retrieve the tile state and occupying actor.

---

## 3. Interaction Flow (Data-Only Resolution)

When a unit interacts with an object:

1.  **Get Unit Location:** Convert the unit's world position to grid coordinates.
2.  **Check Trigger:** Verify the unit is on a tile with `State == Trigger`.
3.  **Resolve Choice:** If the player clicks an object, check if that object's coordinates are in the unit's current tile's `ParentObjectCoordinates` array.
4.  **Execute:** If valid, call `Execute_Interact` on the `OccupyingActor` stored in the target tile's data.

---

## 4. Technical Implementation Notes (C++)

### 4.1 Data Structure (`FTileData`)
Stored in `ShipGridManager.h`. It contains the state, parent coordinates for triggers, and a pointer to the occupying `AActor`.

### 4.2 Visual Refresh
`UpdateTileVisuals(FIntVector Coords)` is called automatically whenever a tile's state changes. It:
*   Removes the existing visual instance (if any).
*   Adds a new instance to the ISMC corresponding to the new state.
*   Updates `LevelControl` tags on the ISMC for visibility management.

### 4.3 Coordinate Conversion
*   `WorldToGrid(FVector)`: Converts world position to grid coordinates.
*   `GridToWorld(FIntVector)`: Converts grid coordinates to the world position (centered on the tile).

---

## 5. Summary of Key Classes

*   **`UShipGridManager`**: The single source of truth for all grid data. Manages ISMCs and `TileDataMap`.
*   **`AGridEntity`**: Base class for interactable ship objects. Snaps to grid and provides trigger-tile detection via `GetInteractionTileData()`.
*   **`ACameraCharacter`**: Translates player clicks into grid coordinates and validates interaction logic against the `ShipGridManager`.
