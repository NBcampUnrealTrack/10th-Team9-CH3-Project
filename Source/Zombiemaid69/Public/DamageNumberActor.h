#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;

UCLASS()
class ZOMBIEMAID69_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	ADamageNumberActor();

	//표시할 데미지 설정
	UFUNCTION(BlueprintCallable, Category = "DamageNumber")
	void SetDamage(float NewDamage);

	//데미지 숫자 이동 및 투명도 처리
	UFUNCTION(BlueprintCallable, Category = "DamageNumber")
	void UpdateDamageNumber(float DeltaTime);

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	//표시할 데미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	float Damage;

	//데미지 숫자 표시 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	float LifeTime;

	//위로 올라가는 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	float MoveSpeed;

	//데미지 숫자 Widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	TObjectPtr<UWidgetComponent> DamageWidgetComponent;

	//생성 후 지난 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DamageNumber")
	float ElapsedTime;
};