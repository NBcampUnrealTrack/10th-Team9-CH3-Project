#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LastCureGameMode.generated.h"

UCLASS(abstract)
class ALastCureGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALastCureGameMode();

	//보스 사망이 확정됐을 때 호출
	//현재 스테이지의 중복 보상 방지하고 원종혈청 지급
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void HandleBossCleared();

protected:
	//해당 스테이지의 보스 처치 시 지급할 원종혈청 수량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SerumReward", meta = (ClampMin = "1"))//ClampMin->에디터에서 입력하는 최솟값을 1로 제한
		int32 BossOriginSerumReward = 1;//기본 보상량 1개
};
