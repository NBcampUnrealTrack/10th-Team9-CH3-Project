#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Zombiemaid69GameInstance.generated.h"

UCLASS()
class ZOMBIEMAID69_API UZombiemaid69GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	//외부에서 원종혈청 추가할 때 호출할 함수(Serum: 혈청)
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void AddOriginSerum(int32 Amount);

	//현재 원종혈청 수량 조회하는 함수 
	UFUNCTION(BlueprintCallable, Category = "Progress")
	int32 GetOriginSerum() const;

private:
	//맵을 변경해도 획득한 원종혈청 유지
	UPROPERTY()
	int32 OriginSerum = 0;
};
