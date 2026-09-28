#include "PauseMenuComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UPauseMenuComponent::UPauseMenuComponent()
{
	// 매 프레임 실행할 작업이 없으므로 Tick을 사용하지 않음
	PrimaryComponentTick.bCanEverTick = false;
}

void UPauseMenuComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPauseMenuComponent::OpenPauseMenu()
{
	// 이 컴포넌트가 붙어 있는 PlayerController 가져오기
	APlayerController* PC = Cast<APlayerController>(GetOwner());

	// 컨트롤러가 없거나 내 화면의 플레이어가 아니면 종료
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// 위젯 클래스가 지정되지 않았다면 종료
	if (!PauseMenuClass)
	{
		return;
	}

	// 메뉴가 이미 생성돼 있다면 중복으로 열지 않음
	if (PauseMenuInstance)
	{
		return;
	}

	// 지정된 클래스로 실제 메뉴 생성
	PauseMenuInstance = CreateWidget<UUserWidget>(PC, PauseMenuClass);

	if (!PauseMenuInstance)
	{
		return;
	}

	// HUD보다 위에 메뉴 표시
	PauseMenuInstance->AddToViewport(100);

	// 게임 일시정지
	UGameplayStatics::SetGamePaused(this, true);

	// 마우스 표시 및 UI 조작 모드로 전환
	PC->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
}

void UPauseMenuComponent::ResumeGame()
{
	// 이 컴포넌트가 붙어 있는 PlayerController 가져오기
	APlayerController* PC = Cast<APlayerController>(GetOwner());

	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// 열린 일시정지 메뉴가 없으면 종료
	if (!PauseMenuInstance)
	{
		return;
	}

	// 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(this, false);

	// 메뉴를 화면에서 제거하고 보관하던 참조 비우기
	PauseMenuInstance->RemoveFromParent();
	PauseMenuInstance = nullptr;

	// 마우스를 숨기고 게임 조작 모드로 복귀
	PC->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
}