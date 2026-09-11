#include "LastCureGameInstance.h"

void ULastCureGameInstance::AddOriginSerum(int32 Amount)
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
int32 ULastCureGameInstance::GetOriginSerum() const
{
	return OriginSerum;
}

//원종혈청 11개 이하 → 제작 실패
bool ULastCureGameInstance::CanCraftVaccine() const
{
	return !bVaccineCreated &&
		OriginSerum >= RequiredOriginSerumForVaccine;
}

//원종혈청 12개 이상 → 12개 소비 → 백신 제작 완료
bool ULastCureGameInstance::TryCraftVaccine()
{
	if (!CanCraftVaccine())
	{
		return false;
	}

	OriginSerum -= RequiredOriginSerumForVaccine;
	bVaccineCreated = true;

	return true;
}

//이미 제작 완료 → 중복 제작 불가
bool ULastCureGameInstance::IsVaccineCreated() const
{
	return bVaccineCreated;
}
