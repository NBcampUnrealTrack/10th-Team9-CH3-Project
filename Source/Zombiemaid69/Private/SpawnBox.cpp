#include "SpawnBox.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"


ASpawnBox::ASpawnBox()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawnPointBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
    SpawnPointBox->SetupAttachment(Scene);
}


FVector ASpawnBox::GetRandomPointInVolume() const
{
    // 박스의 로컬 공간에서 사용할 Extent
    FVector BoxExtent = SpawnPointBox->GetUnscaledBoxExtent();

    // 박스 내부의 랜덤한 로컬 좌표
    FVector RandomLocalPoint(
        FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
        FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
    );

    // 로컬 좌표를 월드 좌표로 변환
    return SpawnPointBox->GetComponentTransform().TransformPosition(RandomLocalPoint);
}

//------------------------------------------------------------------------------------
// 스폰 영역하고 영역내부에서 랜덤 좌표로 스폰되게 만들어 둠 "적 AI 클래스가 확실하게 정해지면" 로직연결하면 됨
//void ASpawnBox::SpawnEnemies()
//{
//    if (EnemyClasses.Num() == 0)
//    {
//        return;
//    }
//
//    for (int32 i = 0; i < SpawnCount; i++)
//    {
//스폰 박스 내부의 랜덤 위치
//        FVector SpawnLocation = GetRandomPointInVolume();
//
//적 종류를 랜덤으로 선택
//        int32 RandomIndex = FMath::RandRange(0, EnemyClasses.Num() - 1);
//
//        TSubclassOf<AEnemyAI> SelectedEnemy = EnemyClasses[RandomIndex];
//
//선택한 적 생성
//       GetWorld()->SpawnActor<AEnemyAI>(
//            SelectedEnemy,
//            SpawnLocation,
//            FRotator::ZeroRotator
//        );
//    }
//}