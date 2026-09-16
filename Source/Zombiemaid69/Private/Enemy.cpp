#include "Enemy.h"
#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "LastCureGameMode.h"
#include "LastCureGameInstance.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	// Enemy를 조종할 AIController 클래스 지정
	AIControllerClass = AEnemyAIController::StaticClass();

	// AIController가 자동으로 Enemy를 조종하도록 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	//스탯
	MaxHealth = 0.0f;
	CurrentHealth = 0.0f;
	AttackDamage = 0.0f;
	AttackRange = 0.0f;
	AttackWidth = 0.0f;
	MoveSpeed = 0.0f;

	//스테이트
	bIsDead = false;

	//보상
	SerumReward = 0;
	ExpReward = 1;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	//현재 맵에서 사용중인 게임모드를 가져옴
	ALastCureGameMode* GM = Cast<ALastCureGameMode>(
		UGameplayStatics::GetGameMode(this)
	);
	
	//게임모드를 정상적으로 가져왔다면 남은 좀비 수를 셀 수 있도록 현재 좀비를 등록
	if (GM)
	{
		GM->RegisterZombie(this);
	}

	CurrentHealth = MaxHealth; // 시작 시 최대 체력에 맞춰 현재 체력을 설정

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; // 적의 이동 속도를 설정된 이동 속도에 맞게 적용

	if (IdleMontage)
	{
		PlayAnimMontage(IdleMontage);
	}
}

//받는 데미지
float AEnemy::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	Super::TakeDamage(
		DamageAmount, 
		DamageEvent, 
		EventInstigator, 
		DamageCauser);

	if (bIsDead) // 이미 사망한 적은 추가 데미지를 받지 않도록 처리
	{
		return 0.0f;
	}

	CurrentHealth -= DamageAmount; // 받은 데미지만큼 현재 체력 감소

	if (CurrentHealth <= 0.0f) // 체력이 0 이하가 되면 사망 처리
	{
		Die();
		return DamageAmount;
	}
	//현재 AIController 가져오기
	AEnemyAIController* AIController = Cast <AEnemyAIController> (GetController());
	if (AIController)
	{
		AActor* Attacker = nullptr;

		//공격자의 Pawn 가져오기
		if (EventInstigator)
		{
			Attacker = EventInstigator->GetPawn();
		}
		//공격자를 찾지 못하면 DamageCauser 사용
		if (!Attacker)
		{
			Attacker = DamageCauser;
		}
		//공격자 추적 처리
		AIController->OnEnemyDamaged(Attacker);
	}

	return DamageAmount;
}

float AEnemy::GetAttackRange() const
{
	return AttackRange;
}

float AEnemy::GetAttackDamage() const
{
	return AttackDamage;
}

float AEnemy::GetAttackWidth() const
{
	return AttackWidth;
}

void AEnemy::RecoverHealth(float DeltaTime)
{
	if (bIsDead)
	{
		return;
	}
	//이미 최대 체력이면 회복하지 않음
	if (CurrentHealth >= MaxHealth)
	{
		return;
	}
	//초당 최대 체력만큼 회복
	CurrentHealth += MaxHealth * DeltaTime;
	
	//최대 체력을 넘지 않도록 제한
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
}

//사망로직
void AEnemy::Die()
{
	if (bIsDead) // 이미 사망 처리된 적이라면 중복으로 처리하지 않음
	{
		return;
	}

	bIsDead = true; // 적을 사망 상태로 변경

	//현재 게임모드를 가져옴
	ALastCureGameMode* GM = Cast<ALastCureGameMode>(
		UGameplayStatics::GetGameMode(this)
	);

	if (GM)
	{
		//현재 좀비가 죽었다고 게임모드에 알림
		GM->HandleZombieDeath(this);
	}

	//현재 게임인스턴스를 가져옴
	ULastCureGameInstance* GI = Cast<ULastCureGameInstance>(
		UGameplayStatics::GetGameInstance(this)
	);

	if (GI)
	{
		//해당 좀비에게 설정된 일반혈청 지급
		GI->AddSerum(SerumReward);
	}
	
	//현재 AIController 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController());

	//AI 사망처리
	if (AIController)
	{
		AIController->OnEnemyDead();
	}
	//사망 애니메이션 재생
	if (DeadMontage)
	{
		PlayAnimMontage(DeadMontage);
	}

	//3초 후 시체 제거
	FTimerHandle DeadBodyTimer;
	GetWorldTimerManager().SetTimer(
		DeadBodyTimer,
		this,
		&AEnemy::RemoveDeadBody,
		2.5f,
		false
	);
}

void AEnemy::RemoveDeadBody()
{
	Destroy();
}

