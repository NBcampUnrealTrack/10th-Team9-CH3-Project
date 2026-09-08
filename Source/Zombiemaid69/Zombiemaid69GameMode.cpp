#include "Zombiemaid69GameMode.h"
#include "Zombiemaid69GameState.h"
#include "Zombiemaid69GameInstance.h"

AZombiemaid69GameMode::AZombiemaid69GameMode()
{
	//이 게임모드가 생성해서 사용할 GameState 클래스 지정
	GameStateClass = AZombiemaid69GameState::StaticClass();
}

void AZombiemaid69GameMode::HandleBossCleared()
{
	//현재 맵의 완료 여부를 관리하는 GameState를 가져옴
	AZombiemaid69GameState* GameState = GetGameState<AZombiemaid69GameState>();

	//맵을 이동한 뒤에도 누적 원종혈청을 유지하는 GameInstance를 가져옴
	UZombiemaid69GameInstance* GameInstance = Cast<UZombiemaid69GameInstance>(GetGameInstance());


	//클래스 설정이 잘못되는 등의 이유로 가져오지 못한다면
	//잘못된 포인터에 접근하지 않도록 처리 종료
	if (!GameState || !GameInstance)
	{
		return;
	}

	//이번 맵 입장에서 이미 완료처리했다면 중복 보상을 지급하지 않음
	//OpenLevel로 재입장하면 GameState가 새로 생성되어 다시 지급 가능
	if (GameState->IsStageCleared())
	{
		return;
	}

	//중복지급 방지하기 위해 먼저 스테이지 클리어 처리(테스트용)
	GameState->MarkStageCleared();
	//원종혈청 지급(테스트용)
	GameInstance->AddOriginSerum(1);
}