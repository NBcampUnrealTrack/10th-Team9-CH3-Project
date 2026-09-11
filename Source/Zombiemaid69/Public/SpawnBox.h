#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnBox.generated.h"

class UBoxComponent;
// 임시 class AEnemy; (나중에 스폰할 적 AI 함수 TSubclassOf<AZombieAI> 이름 정해지면 교체하면 사용가능) "변경완료"

UCLASS()
class ZOMBIEMAID69_API ASpawnBox : public AActor
{
	GENERATED_BODY()

public:
	ASpawnBox();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	USceneComponent* Scene;

	// 스폰 영역 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	UBoxComponent* SpawnPointBox;

	// 스폰 영역 내부에서 무작위 좌표를 가져오는 함수
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomPointInVolume() const;

protected:
	virtual void BeginPlay() override;

};
	//------------------------------나중에 수정하면 됨---------------------------------------

	// 스폰할 적 AI 함수
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	// 임시 <AZombieAI>를 부모로 하는 클래스 중에서 선택해서 넣을 수 있는 (나중에 C++ 클래스 정해지면 이름만 교체하면 사용가능) "변경완료"
	// TSubclassOf<AEnemy> EnemyClass;

	// 스폰할 수 있는 적 AI 클래스 목록 <이건 고민중임>
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	// TArray<TSubclassOf<AEnemy>> EnemyClasses;

	// 생성할 적의 수 함수
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	// SpawnCount = 1;은 스폰 영역 내부에 한 마리만 생성하겠다는 뜻
	// int32 SpawnCount = 1;

	// 적 AI 생성 함수
	// UFUNCTION(BlueprintCallable, Category = "Spawn")
	// void SpawnEnemies();

	
