// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelControl.h"
#include "EngineUtils.h"
#include "TheSeeker.h"
#include "Components/InstancedStaticMeshComponent.h"

const FName ALevelControl::TAG_MULTI_FLOOR(TEXT("MultiFloor"));
const FString ALevelControl::PREFIX_FLOOR(TEXT("Floor_"));

ALevelControl::ALevelControl() {
	PrimaryActorTick.bCanEverTick = false;
}

void ALevelControl::BeginPlay() {
	Super::BeginPlay();
	
	// Initial scan for multi-floor actors in the scene
	for (TActorIterator<AActor> it(GetWorld()); it; ++it) {
		if (it->ActorHasTag(TAG_MULTI_FLOOR)) {
			RegisterMultiFloorActor(*it);
		}
	}

	UpdateFloorVisibility();
}

void ALevelControl::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

ALevelControl* ALevelControl::FindLevelControl(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	for (TObjectIterator<ALevelControl> It; It; ++It)
	{
		if (It->GetWorld() == World) return *It;
	}
	return nullptr;
}

void ALevelControl::ApplyFloorTags(AActor* Actor, int32 Floor)
{
	if (!Actor) return;

	if (!Actor->ActorHasTag(TAG_MULTI_FLOOR)) {
		Actor->Tags.Add(TAG_MULTI_FLOOR);
	}

	for (int32 i = Actor->Tags.Num() - 1; i >= 0; --i) {
		if (Actor->Tags[i].ToString().StartsWith(PREFIX_FLOOR)) {
			Actor->Tags.RemoveAt(i);
		}
	}

	Actor->Tags.Add(FName(*FString::Printf(TEXT("%s%d"), *PREFIX_FLOOR, Floor)));
}

void ALevelControl::SetFloor(int32_t floor) {
	if (CURRENT_FLOOR == floor) return;
	
	CURRENT_FLOOR = floor;
	UpdateFloorVisibility();
}

void ALevelControl::FloorUp() {
	if (CURRENT_FLOOR == MAX_FLOOR) return;
	SetFloor(CURRENT_FLOOR + 1);
}

void ALevelControl::FloorDown() {
	if (CURRENT_FLOOR == MIN_FLOOR) return;
	SetFloor(CURRENT_FLOOR - 1);
}

void ALevelControl::RegisterMultiFloorActor(AActor* Actor) {
	if (Actor && !MultiFloorActors.Contains(Actor)) {
		MultiFloorActors.Add(Actor);

		for (const FName& Tag : Actor->Tags) {
			FString TagStr = Tag.ToString();
			if (TagStr.StartsWith(PREFIX_FLOOR)) {
				int32 FloorIdx = FCString::Atoi(*TagStr.RightChop(PREFIX_FLOOR.Len()));
				if (FloorIdx > MAX_FLOOR) {
					MAX_FLOOR = FloorIdx;
				}
				break;
			}
		}

		UpdateFloorVisibility();
	}
}

void ALevelControl::UpdateFloorVisibility() const {
	for (AActor* actor : MultiFloorActors) {
		if (!actor) continue;

		int32 itemFloor = -1;
		for (const FName& Tag : actor->Tags) {
			FString TagStr = Tag.ToString();
			if (TagStr.StartsWith(PREFIX_FLOOR)) {
				itemFloor = FCString::Atoi(*TagStr.RightChop(PREFIX_FLOOR.Len()));
				break;
			}
		}

		if (itemFloor != -1) {
			bool bShouldHide = itemFloor > CURRENT_FLOOR;
			
			actor->SetActorHiddenInGame(bShouldHide);

			TArray<UPrimitiveComponent*> Comps;
			actor->GetComponents<UPrimitiveComponent>(Comps);
			
			for (UPrimitiveComponent* Comp : Comps) {
				Comp->SetCollisionResponseToChannel(
					COLLISION_FLOOR, 
					bShouldHide ? ECR_Ignore : ECR_Block
				);
				
				Comp->SetCollisionResponseToChannel(
					ECC_Visibility, 
					bShouldHide ? ECR_Ignore : ECR_Block
				);
			}
		}

		TArray<UInstancedStaticMeshComponent*> GridISMCs;
		actor->GetComponents<UInstancedStaticMeshComponent>(GridISMCs);
		for (UInstancedStaticMeshComponent* ISMC : GridISMCs) {
			if (ISMC->ComponentHasTag(TAG_MULTI_FLOOR)) {
				int32 compFloor = -1;
				for (const FName& Tag : ISMC->ComponentTags) {
					FString TagStr = Tag.ToString();
					if (TagStr.StartsWith(PREFIX_FLOOR)) {
						compFloor = FCString::Atoi(*TagStr.RightChop(PREFIX_FLOOR.Len()));
						break;
					}
				}

				if (compFloor != -1) {
					bool bHideComp = compFloor > CURRENT_FLOOR;

					ISMC->SetVisibility(!bHideComp);
					ISMC->SetCollisionResponseToChannel(COLLISION_FLOOR, bHideComp ? ECR_Ignore : ECR_Block);
					ISMC->SetCollisionResponseToChannel(ECC_Visibility, bHideComp ? ECR_Ignore : ECR_Block);
				}
			}
		}
	}
}
