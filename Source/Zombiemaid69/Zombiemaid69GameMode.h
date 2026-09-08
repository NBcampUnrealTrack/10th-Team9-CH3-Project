#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Zombiemaid69GameMode.generated.h"

UCLASS(abstract)
class AZombiemaid69GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZombiemaid69GameMode();

	//보스 사망이 확정됐을 때 호출
	//현재 스테이지의 중복 보상 방지하고 원종혈청 지급
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void HandleBossCleared();
};



