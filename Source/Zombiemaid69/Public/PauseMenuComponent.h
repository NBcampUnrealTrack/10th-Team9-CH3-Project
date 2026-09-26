#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PauseMenuComponent.generated.h"

// 여기서는 위젯 클래스의 이름만 알려줌
class UUserWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ZOMBIEMAID69_API UPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPauseMenuComponent();

	// 블루프린트에서 호출할 일시정지 메뉴 열기
	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void OpenPauseMenu();

	// 블루프린트에서 호출할 게임 재개
	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void ResumeGame();

protected:
	virtual void BeginPlay() override;

	// 에디터에서 WBP_PauseMenu를 지정할 자리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Pause")
	TSubclassOf<UUserWidget> PauseMenuClass;

private:
	// 생성한 메뉴를 보관해 두었다가 닫을 때 사용
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuInstance = nullptr;
};