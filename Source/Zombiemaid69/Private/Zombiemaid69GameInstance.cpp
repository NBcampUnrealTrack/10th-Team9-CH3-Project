#include "Zombiemaid69GameInstance.h"

void UZombiemaid69GameInstance::AddOriginSerum(int32 Amount)
{
	//추가할 원종혈청 수량이 0 이하면 함수 종료
	if (Amount <= 0)
	{
		return;
	}

	//검사를 통과한 양수만 누적
	OriginSerum += Amount;
}

//현재 원종혈청 수량 반환
int32 UZombiemaid69GameInstance::GetOriginSerum() const
{
	return OriginSerum;
}