#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Zombiemaid69GameState.generated.h"

UCLASS()
class ZOMBIEMAID69_API AZombiemaid69GameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsStageCleared() const;

	void MarkStageCleared();

private:
	UPROPERTY()
	bool bStageCleared = false;
};
