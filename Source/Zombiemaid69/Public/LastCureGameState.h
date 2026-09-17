#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LastCureGameState.generated.h"

//남은 좀비 수가 변경됐을 때 UI에 알려주는 이벤트 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnRemainingZombieCountChanged,
    int32,
    NewCount
);

//현재 맴 입장 동안 스테이지 완료 상태 관리
UCLASS()
class ZOMBIEMAID69_API ALastCureGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	//현재 스테이지 완료 여부 조회
	UFUNCTION(BlueprintPure, Category = "Stage")
	bool IsStageCleared() const;

	//현재 스테이지를 완료 상태로 변경
	void MarkStageCleared();

	//현재 일반 맵에 남아있는 좀비 수 조회
	UFUNCTION(BlueprintPure, Category = "Stage")
	int32 GetRemainingZombieCount() const;

	//게임모드에서 집계한 남은 좀비 수 반영
	void SetRemainingZombieCount(int32 NewCount);

	//블루프린트에서 남은 좀비 수 변경 이벤트를 받을 수 있게 함
	UPROPERTY(BlueprintAssignable, Category = "Stage")
	FOnRemainingZombieCountChanged OnRemainingZombieCountChanged;

private:
	//이번 맵 입장에서 완료 처리가 이뤄졌는지 확인
	//GameMode가 이 값을 확인해 보상 중복 지급을 방지
	//OpenLevel로 맵을 새로 불러오면 새로운 GameState에서 false로 시작
	UPROPERTY()
	bool bStageCleared = false;

	//일반 맵의 좀비 전멸 진행도
	UPROPERTY()
	int32 RemainingZombieCount = 0;
};
