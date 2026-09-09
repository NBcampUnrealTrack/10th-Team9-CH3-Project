#include "LastCureGameState.h"

bool ALastCureGameState::IsStageCleared() const
{
	//상태를 변경하지 않고 현재 완료 여부를 반환
	return bStageCleared;
}

void ALastCureGameState::MarkStageCleared()
{
	//이후 보스 처치 알림에서 중복 보상을 막을 수 있도록 완료 표시
	bStageCleared = true;
}

int32 ALastCureGameState::GetRemainingZombieCount() const
{
	//현재 남은 좀비 수 반환
	return RemainingZombieCount;
}

void ALastCureGameState::SetRemainingZombieCount(int32 NewCount)
{
	//잘못된 값이 전달되어도 남은 수가 음수가 되지 않도록 제한
	RemainingZombieCount = FMath::Max(0, NewCount);
}