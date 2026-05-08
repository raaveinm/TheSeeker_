//
// Created by raaveinm on 2/19/26.
//
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelControl.generated.h"

UCLASS()
class THESEEKER_API ALevelControl : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelControl();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	///////////////////////////////////////////////
	/// Floor Control System
	/////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LevelControl")
	int32 CURRENT_FLOOR = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LevelControl")
	int32 MAX_FLOOR = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LevelControl")
	int32 MIN_FLOOR = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LevelControl")
	float FLOOR_HEIGHT = 300.f;
	
	UFUNCTION(BlueprintCallable, Category="LevelControl")
	void SetFloor(const int32 floor);
	
	UFUNCTION(BlueprintCallable, Category="LevelControl")
	int32 GetFloor() const { return CURRENT_FLOOR; }
	
	UFUNCTION(BlueprintCallable, Category="LevelControl")
	void FloorUp();
	
	UFUNCTION(BlueprintCallable, Category="LevelControl")
	void FloorDown();

	UFUNCTION(BlueprintCallable, Category = "LevelControl")
	void RegisterMultiFloorActor(AActor* Actor);

	// Static helper to find the Level Control in the world
	UFUNCTION(BlueprintPure, Category = "LevelControl", meta = (WorldContext = "WorldContextObject"))
	static ALevelControl* FindLevelControl(UObject* WorldContextObject);

	// Static helper to apply standardized tags to an actor
	UFUNCTION(BlueprintCallable, Category = "LevelControl")
	static void ApplyFloorTags(AActor* Actor, int32 Floor);

	static const FName TAG_MULTI_FLOOR;
	static const FString PREFIX_FLOOR;
	
	UFUNCTION(BlueprintCallable, Category = "LevelControl")
	void UpdateFloorVisibility() const;

private:
	UPROPERTY()
	TArray<AActor*> MultiFloorActors;
	
};
