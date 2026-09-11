#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LastCureGameInstance.generated.h"

UCLASS()
class ZOMBIEMAID69_API ULastCureGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//외부에서 원종혈청 추가할 때 호출할 함수(Serum: 혈청)
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void AddOriginSerum(int32 Amount);

	//현재 원종혈청 수량 조회하는 함수 
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetOriginSerum() const;

	//현재 원종혈청으로 백신을 제작할 수 있는지 확인
	UFUNCTION(BlueprintPure, Category = "Progress")
	bool CanCraftVaccine() const;

	//조건을 만족하면 원종혈청을 사용하여 백신 제작
	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool TryCraftVaccine();

	//백신 제작 완료 여부 조회
	UFUNCTION(BlueprintPure, Category = "Progress")
	bool IsVaccineCreated() const;

private:
	//맵을 변경해도 획득한 원종혈청 유지
	UPROPERTY()
	int32 OriginSerum = 0;

	//백신 제작에 필요한 원종혈청 수량
	static constexpr int32 RequiredOriginSerumForVaccine = 12;

	//백신 제작 완료 여부
	UPROPERTY()
	bool bVaccineCreated = false;
};
