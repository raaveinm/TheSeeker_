//
// Created by raaveinm on 2/19/26.
//

#include "LevelControl.h"
#include "EngineUtils.h"

ALevelControl::ALevelControl() {
	PrimaryActorTick.bCanEverTick = false;

}

void ALevelControl::BeginPlay() {
	Super::BeginPlay();
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

void ALevelControl::UpdateFloorVisibility() const {
	for (TActorIterator<AActor> it(GetWorld()); it; ++it) {
		AActor* actor = *it;
		if (actor->ActorHasTag(FName("MultiFloor"))) {
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
						ECC_GameTraceChannel1, 
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
}
