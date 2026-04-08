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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LevelControl")
	int32 CURRENT_FLOOR = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LevelControl")
	int32 MAX_FLOOR = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LevelControl")
	int32 MIN_FLOOR = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="LevelControl")
	float FLOOR_HEIGHT = 200.f;
	
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

	static const FName TAG_MULTI_FLOOR;
	
private:
	void UpdateFloorVisibility() const;

	UPROPERTY()
	TArray<AActor*> MultiFloorActors;
	
};
