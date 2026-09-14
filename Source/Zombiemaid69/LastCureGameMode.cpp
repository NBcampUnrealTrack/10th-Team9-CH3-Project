#include "LastCureGameMode.h"
#include "LastCureGameState.h"
#include "LastCureGameInstance.h"
#include "Kismet/GameplayStatics.h"

ALastCureGameMode::ALastCureGameMode()
{
	//이 게임모드가 생성해서 사용할 GameState 클래스 지정
	GameStateClass = ALastCureGameState::StaticClass();
}

void ALastCureGameMode::HandleBossCleared()
{
	//현재 맵의 완료 여부를 관리하는 GameState를 가져옴
	ALastCureGameState* GS = GetGameState<ALastCureGameState>();

	//맵을 이동한 뒤에도 누적 원종혈청을 유지하는 GameInstance를 가져옴
	ULastCureGameInstance* GI = Cast<ULastCureGameInstance>(GetGameInstance());


	//클래스 설정이 잘못되는 등의 이유로 가져오지 못한다면
	//잘못된 포인터에 접근하지 않도록 처리 종료
	if (!GS || !GI)
	{
		return;
	}

	//이번 맵 입장에서 이미 완료처리했다면 중복 보상을 지급하지 않음
	//OpenLevel로 재입장하면 GameState가 새로 생성되어 다시 지급 가능
	if (GS->IsStageCleared())
	{
		return;
	}

	//중복지급 방지하기 위해 먼저 스테이지 클리어 처리(테스트용)
	GS->MarkStageCleared();
	//원종혈청 지급(테스트용)
	GI->AddOriginSerum(BossOriginSerumReward);
}

void ALastCureGameMode::RegisterZombie(AActor* Zombie)
{
	//유효하지 않은 액터는 등록하지 않음
	if (!IsValid(Zombie))
	{
		return;
	}

	ALastCureGameState* GS = GetGameState<ALastCureGameState>();

	if (!GS)
	{
		return;
	}

	//목록에 보관할 약한참조 생성
	const TWeakObjectPtr<AActor> ZombieReference(Zombie);

	//이미 등록된 좀비라면 중복 처리하지 않음
	if (RemainingZombies.Contains(ZombieReference))
	{
		return;
	}
	RemainingZombies.Add(ZombieReference);

	//등록된 좀비 수를 GameState에 반영
	GS->SetRemainingZombieCount(RemainingZombies.Num());
}

void ALastCureGameMode::HandleZombieDeath(AActor* Zombie)
{
	if (!IsValid(Zombie))
	{
		return;
	}

	ALastCureGameState* GS = GetGameState<ALastCureGameState>();
	if (!GS)
	{
		return;
	}
	
	const TWeakObjectPtr<AActor> ZombieReference(Zombie);

	//등록되지 않았거나 이미 처리한 좀비라면 무시
	if (RemainingZombies.Remove(ZombieReference) == 0)
	{
		return;
	}

	//제거 후 남은 좀비 수를 GameState에 반영
	GS->SetRemainingZombieCount(RemainingZombies.Num());

	//마지막 좀비를 처치했다면 보스방 입장 가능 상태로 변경
	if (RemainingZombies.Num() == 0)
	{
		//보스방 잠금 해제
		bAllZombiesKilled = true;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("모든 좀비를 처치했습니다. 보스방 입장이 가능합니다.")
		);

		return;
	}

	//테스트용 UE_LOG
	UE_LOG(
		LogTemp,
		Display,
		TEXT("남은 좀비 수: %d"),
		GS->GetRemainingZombieCount()
	);
}

bool ALastCureGameMode::CanEnterBossRoom() const
{
	return bAllZombiesKilled && !BossLevel.IsNull();//모든 좀비를 처치했고 이동할 보스맵도 설정되어있을 때 보스방에 입장 가능
}

void ALastCureGameMode::EnterBossRoom()
{
	if (!CanEnterBossRoom())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("보스방 입장 조건을 만족하지 않았습니다.")
		);
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, BossLevel);//현재 맵을 종료하고 BossLevel로 설정된 맵을 불러옴
}

void ALastCureGameMode::ReturnToLaboratory()
{
	if (LaboratoryLevel.IsNull())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("돌아갈 연구실 맵이 설정되지 않았습니다.")
		);
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LaboratoryLevel);
}

void ALastCureGameMode::EnterStage(TSoftObjectPtr<UWorld> StageLevel)
{
	if (StageLevel.IsNull())
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("이동할 일반 스테이지가 설정되지 않았습니다.")
		);
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, StageLevel);
}

void ALastCureGameMode::HandlePlayerDeath()
{
	//동일한 사망 처리가 다시 들어오면 무시
	if (bPlayerDeathHandled)
	{
		return;
	}

	ULastCureGameInstance* GI =
		Cast<ULastCureGameInstance>(GetGameInstance());

	if (!GI)
	{
		return;
	}
	bPlayerDeathHandled = true;
	GI->ApplySerumDeathPenalty();

	//테스트용 UE_Log
	UE_LOG(
		LogTemp,
		Display,
		TEXT("사망 후 일반 혈청 : %d"),
		GI->GetSerum()
	);
}