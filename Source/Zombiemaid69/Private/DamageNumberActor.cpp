#include "DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

ADamageNumberActor::ADamageNumberActor()
{
	//Tick 사용
	PrimaryActorTick.bCanEverTick = true;

	//데미지 기본값
	Damage = 0.0f;

	//기본 표시 시간
	LifeTime = 1.0f;

	//위로 올라가는 속도
	MoveSpeed = 50.0f;

	//지난 시간 초기화
	ElapsedTime = 0.0f;
}

void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();

	//Blueprint의 Widget Component 찾기
	DamageWidgetComponent = FindComponentByClass<UWidgetComponent>();

	//일정 시간 후 자동 제거
	SetLifeSpan(LifeTime);
}

void ADamageNumberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//데미지 숫자 연출 처리
	UpdateDamageNumber(DeltaTime);
}

void ADamageNumberActor::SetDamage(float NewDamage)
{
	//표시할 데미지 저장
	Damage = NewDamage;
}

void ADamageNumberActor::UpdateDamageNumber(float DeltaTime)
{
	//시간에 맞춰 위로 이동
	AddActorWorldOffset(
		FVector(0.0f, 0.0f, MoveSpeed * DeltaTime)
	);

	//지난 시간 누적
	ElapsedTime += DeltaTime;

	if (!DamageWidgetComponent || LifeTime <= 0.0f)
	{
		return;
	}

	//실제 생성된 UserWidget 가져오기
	UUserWidget* UserWidget = DamageWidgetComponent->GetUserWidgetObject();

	if (!UserWidget)
	{
		return;
	}

	//남은 시간에 맞춰 투명도 감소
	const float Opacity =
		FMath::Clamp(1.0f - (ElapsedTime / LifeTime), 0.0f, 1.0f);

	//실제 Widget 전체 투명도 변경
	UserWidget->SetRenderOpacity(Opacity);
}