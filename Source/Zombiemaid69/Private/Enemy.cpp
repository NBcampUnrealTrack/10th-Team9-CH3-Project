#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"

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
	
	CurrentHealth = MaxHealth; // 시작 시 최대 체력에 맞춰 현재 체력을 설정

	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; // 적의 이동 속도를 설정된 이동 속도에 맞게 적용
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
	}

	return DamageAmount;
}

//사망로직
void AEnemy::Die()
{
	if (bIsDead) // 이미 사망 처리된 적이라면 중복으로 처리하지 않음
	{
		return;
	}

	bIsDead = true; // 적을 사망 상태로 변경
	//사망처리
}

