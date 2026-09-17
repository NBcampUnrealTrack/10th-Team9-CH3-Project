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
	//남은 좀비 수가 음수가 되지 않도록 제한
	const int32 SafeCount = FMath::Max(0, NewCount);

	//이전 수량과 같으면 UI에 다시 알리지 않음
	if (RemainingZombieCount == SafeCount)
	{
		return;
	}

	RemainingZombieCount = SafeCount;

	//변경된 수량을 UI에 알림
	OnRemainingZombieCountChanged.Broadcast(RemainingZombieCount);
}