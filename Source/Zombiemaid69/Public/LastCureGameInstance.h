#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LastCureGameInstance.generated.h"

// 일반 혈청 수량이 변경됐을 때 UI에 알려주는 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSerumChanged);

UCLASS()
class ZOMBIEMAID69_API ULastCureGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	// 일반 혈청 변경 알림
	// 블루프린트에서 이 이벤트에 UpdateSerumHUD를 연결
	UPROPERTY(BlueprintAssignable, Category = "Resources|Serum")
	FOnSerumChanged OnSerumChanged;

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

	//일반 혈청 획득
	UFUNCTION(BlueprintCallable, Category = "Resources|Serum")
	void AddSerum(int32 Amount);

	//현재 일반 혈청 수량 조회
	UFUNCTION(BlueprintPure, Category = "Resources|Serum")
	int32 GetSerum() const;

	//상점 가격을 지불할 수 있는지 확인
	UFUNCTION(BlueprintPure, Category = "Resources|Serum")
	bool CanAffordSerum(int32 Cost) const;

	//상점 구매 시 혈청 소비
	UFUNCTION(BlueprintCallable, Category = "Resources|Serum")
	bool SpendSerum(int32 Cost);

	//플레이어 사망 시 혈청 50% 감소
	UFUNCTION(BlueprintCallable, Category = "Resources|Serum")
	void ApplySerumDeathPenalty();

private:
	//맵을 변경해도 획득한 원종혈청 유지
	UPROPERTY()
	int32 OriginSerum = 0;

	//백신 제작에 필요한 원종혈청 수량
	static constexpr int32 RequiredOriginSerumForVaccine = 12;

	//백신 제작 완료 여부
	UPROPERTY()
	bool bVaccineCreated = false;

	//상점 구매에 사용하는 일반 혈청
	//최대보유량 제한 없음
	UPROPERTY()
	int32 Serum = 0;
};
