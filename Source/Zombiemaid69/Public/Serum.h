#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Serum.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

UCLASS()
class ZOMBIEMAID69_API ASerum : public AActor
{
	GENERATED_BODY()

public:
	ASerum();

	//혈청 획득량 설정
	UFUNCTION(BlueprintCallable, Category = "Serum")
	void SetSerumAmount(int32 NewAmount);

	//혈청 획득량 확인
	UFUNCTION(BlueprintPure, Category = "Serum")
	int32 GetSerumAmount() const { return SerumAmount; }

	//혈청 회전
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	//플레이어 획득 판정
	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	//혈청 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Serum")
	TObjectPtr<UStaticMeshComponent> SerumMesh;

	//획득 범위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Serum")
	TObjectPtr<USphereComponent> PickupSphere;

	//획득할 혈청량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Serum")
	int32 SerumAmount;

	//회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Serum|Effect")
	float RotationSpeed;
	//처음 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Serum|Effect")
	FVector StartLocation;
	//위아래 이동 높이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Serum|Effect")
	float FloatHeight;
	//위아래 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Serum|Effect")
	float FloatSpeed;
};