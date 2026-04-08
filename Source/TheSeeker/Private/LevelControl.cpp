//
// Created by raaveinm on 2/19/26.
//

#include "LevelControl.h"
#include "EngineUtils.h"
#include "TheSeeker.h"

const FName ALevelControl::TAG_MULTI_FLOOR(TEXT("MultiFloor"));

ALevelControl::ALevelControl() {
	PrimaryActorTick.bCanEverTick = false;
}

void ALevelControl::BeginPlay() {
	Super::BeginPlay();
	
	// Initial scan for multi-floor actors
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
	}
}

void ALevelControl::UpdateFloorVisibility() const {
	for (AActor* actor : MultiFloorActors) {
		if (!actor) continue;

		int32 itemFloor = -1;
		for (int floor = 0; floor <= MAX_FLOOR; ++floor) {
			FName tag = FName(*FString::Printf(TEXT("Floor_%d"), floor));
			if (actor->ActorHasTag(tag))
			{
				itemFloor = floor;
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
	}
}
