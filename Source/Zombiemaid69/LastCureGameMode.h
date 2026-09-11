#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LastCureGameMode.generated.h"

UCLASS(abstract)
class ZOMBIEMAID69_API ALastCureGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALastCureGameMode();

	//보스 사망이 확정됐을 때 호출
	//현재 스테이지의 중복 보상 방지하고 원종혈청 지급
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void HandleBossCleared();

	//일반 맵 처치대상 좀비 등록
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void RegisterZombie(AActor* Zombie);

	//등록된 좀비가 죽었을 때 호출
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void HandleZombieDeath(AActor* Zombie);

	//보스방에 입장할 수 있는 조건인지 반환
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool CanEnterBossRoom() const;

	//조건을 만족했을 때 해당 보스맵으로 이동
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void EnterBossRoom();

	//현재 맵을 종료하고 연구실로 이동
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ReturnToLaboratory();

	//연구실에서 선택한 일반 스테이지로 이동
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void EnterStage(TSoftObjectPtr<UWorld> StageLevel);

	//플레이어 사망 처리하고 혈청 패널티 적용
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void HandlePlayerDeath();

protected:
	//해당 스테이지의 보스 처치 시 지급할 원종혈청 수량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SerumReward", meta = (ClampMin = "1"))//ClampMin->에디터에서 입력하는 최솟값을 1로 제한
		int32 BossOriginSerumReward = 1;//기본 보상량 1개

	// 일반맵의 좀비를 모두 처치했을 때 이동할 보스맵
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	TSoftObjectPtr<UWorld> BossLevel;

	//보스 처치 또는 플레이어 사망 후 돌아갈 연구실 맵
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage")
	TSoftObjectPtr<UWorld> LaboratoryLevel;


private:
	TSet<TWeakObjectPtr<AActor>> RemainingZombies;

	bool bAllZombiesKilled = false;

	//동일한 사망에서 패널티가 여러번 적용되는것을 방지
	bool bPlayerDeathHandled = false;
};
