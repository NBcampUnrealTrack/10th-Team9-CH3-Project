#include "HealingComponent.h"
#include "StatsComponent.h"
#include "GameFramework/Actor.h"

UHealingComponent::UHealingComponent()
{
	// 타이머 기반으로만 동작
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealingComponent::BeginPlay()
{
	Super::BeginPlay();

	// 같은 액터에 붙어있는 StatsComponent를 찾아서 캐싱
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		CachedStatsComponent = OwnerActor->FindComponentByClass<UStatsComponent>();
	}

	// 초기 아이템 개수를 UI에 알림
	OnItemCountChanged.Broadcast(BandageCount, SyringeCount);
}

void UHealingComponent::UseBandage()
{
	// 이미 다른 아이템을 사용 중이면 중복 사용 방지
	if (bIsUsingItem)
	{
		return;
	}

	// 보유한 붕대가 없으면 사용 불가
	if (BandageCount <= 0)
	{
		// TODO: 아이템 없음 알림 사운드/UI 재생 코드 추가 예정
		return;
	}

	// StatsComponent가 없거나 이미 사망 상태라면 사용 불가
	if (!CachedStatsComponent || !CachedStatsComponent->IsAlive())
	{
		return;
	}

	// 이미 체력이 가득 찬 상태라면 굳이 사용하지 않도록 막음 (원치 않으면 이 체크는 삭제 가능)
	if (CachedStatsComponent->CurrentHealth >= CachedStatsComponent->MaxHealth)
	{
		return;
	}

	// 붕대 소모
	BandageCount--;

	// 소모된 개수를 UI에 알림
	OnItemCountChanged.Broadcast(BandageCount, SyringeCount);

	// 공통 사용 로직 시작 (회복량, 사용 시간을 붕대 기준으로 전달)
	StartUsingItem(BandageHealAmount, BandageUseDuration);
}

void UHealingComponent::UseSyringe()
{
	// 이미 다른 아이템을 사용 중이면 중복 사용 방지
	if (bIsUsingItem)
	{
		return;
	}

	// 보유한 주사기가 없으면 사용 불가
	if (SyringeCount <= 0)
	{
		// TODO: 아이템 없음 알림 사운드/UI 재생 코드 추가 예정
		return;
	}

	// StatsComponent가 없거나 이미 사망 상태라면 사용 불가
	if (!CachedStatsComponent || !CachedStatsComponent->IsAlive())
	{
		return;
	}

	// 이미 체력이 가득 찬 상태라면 굳이 사용하지 않도록 막음
	if (CachedStatsComponent->CurrentHealth >= CachedStatsComponent->MaxHealth)
	{
		return;
	}

	// 주사기 소모
	SyringeCount--;

	// 소모된 개수를 UI에 알림
	OnItemCountChanged.Broadcast(BandageCount, SyringeCount);

	// 공통 사용 로직 시작 (회복량, 사용 시간을 주사기 기준으로 전달)
	StartUsingItem(SyringeHealAmount, SyringeUseDuration);
}

void UHealingComponent::StartUsingItem(float HealAmount, float UseDuration)
{
	// 사용 중 상태로 전환 (캐릭터 Tick이 이 값을 읽어 이동속도를 줄임)
	bIsUsingItem = true;

	// 타이머 콜백에서 사용할 회복량을 미리 저장
	PendingHealAmount = HealAmount;

	// 아이템 사용을 시작했다는 이벤트 브로드캐스트 (애니메이션 재생 등에서 구독 가능)
	OnHealingStart.Broadcast();

	// TODO: 붕대/주사기 사용 애니메이션 및 사운드 재생 코드 추가 예정

	// UseDuration초 후에 FinishUsingItem()이 자동 호출되도록 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		HealingTimerHandle,
		this,
		&UHealingComponent::FinishUsingItem,
		UseDuration,
		false
	);
}

void UHealingComponent::FinishUsingItem()
{
	// 저장해둔 회복량만큼 실제 체력 회복 적용
	if (CachedStatsComponent)
	{
		CachedStatsComponent->HealHealth(PendingHealAmount);
	}

	// 사용 중 상태 해제 (캐릭터 Tick이 이걸 감지해서 이동속도를 원래대로 복원)
	bIsUsingItem = false;

	// 아이템 사용이 끝났다는 이벤트 브로드캐스트
	OnHealingEnd.Broadcast();
}