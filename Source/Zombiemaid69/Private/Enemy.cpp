#include "Enemy.h"
#include "EnemyAIController.h"
#include "DamageNumberActor.h"
#include "Serum.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
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

	//총기 Visibility Trace가 Enemy를 피격할 수 있도록 설정
	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Visibility,
		ECR_Block
	);

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

	//최초 감지 범위 기본값
	DetectionRange = 1000.0f;

	//360도 범위 감지용 Sphere 생성
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	//Enemy의 Capsule에 감지 Sphere 부착
	DetectionSphere->SetupAttachment(GetCapsuleComponent());
	//감지 범위 설정
	DetectionSphere->SetSphereRadius(DetectionRange);
	//감지 전용이므로 물리 충돌은 사용하지 않음
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//기본 충돌 반응은 모두 무시
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	//Pawn만 감지
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//OverLap 이벤트 활성화
	DetectionSphere->SetGenerateOverlapEvents(true);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	//감지 범위 적용
	DetectionSphere->SetSphereRadius(DetectionRange);
	//감지 범위 진입 이벤트 연결
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemy::OnDetectionBeginOverlap
	);
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

void AEnemy::OnDetectionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, 
	const FHitResult& SweepResult)
{
	//감지된 대상이 없으면 처리하지 않음
	if (!OtherActor)
	{
		return;
	}
	//현재 Enemy의 AIController 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController());
	if (!AIController)
	{
		return;
	}
	//감지된 대상을 AIController에 전달
	AIController->OnTargetDetected(OtherActor);
}

//받는 데미지 - 추가수정(윤민)
float AEnemy::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	// 시체, 체력0인 경우, 피해가 없는 요청 무시
	if (!IsAlive() || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}
	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);
	if (!IsAlive() || ActualDamage <= 0.0f)
	{
		return 0.0f;
	}
	const float PreviousHealth = CurrentHealth;

	// 체력이 음수가 되지않게 제한
	CurrentHealth = FMath::Max(
		PreviousHealth - ActualDamage,
		0.0f
	);
	const float HealthLost = PreviousHealth - CurrentHealth;
	if (HealthLost <= 0.0f)
	{
		return 0.0f;
	}
	//실제로 받은 데미지 표시
	ShowDamageNumber(HealthLost);

	// 치명타도 알림 보내고 사망처리
	OnHealthChanged.Broadcast(
		this,
		PreviousHealth,
		CurrentHealth,
		EventInstigator,
		DamageCauser
	);
	if (!IsAlive())
	{
		Die();
		return HealthLost;
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
	return HealthLost;
}

void AEnemy::CheckDetectionTargets()
{
	//감지 Sphere가 없으면 처리하지않음
	if (!DetectionSphere)
	{
		return;
	}
	//현재 AIController 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(GetController());
	if (!AIController)
	{
		return;
	}
	//현재 감지 범위 안의 Actor 가져오기
	TArray<AActor*> OverlappingActors;
	DetectionSphere->GetOverlappingActors(OverlappingActors);

	//범위 안에 대상 다시 확인
	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!OverlappingActor)
		{
			continue;
		}
		//기존 감지 로직으로 전달
		AIController->OnTargetDetected(OverlappingActor);
	}
}

void AEnemy::ShowDamageNumber(float Damage)
{
	//데미지 숫자 Actor가 없으면 생성하지않음
	if (!DamageNumberClass)
	{
		return;
	}
	//좀비 위쪽에 생성
	const FVector SpawnLocation =
		GetActorLocation() + FVector(0.0f, 0.0f, 150.0f);
	const FTransform SpawnTransform(
		GetActorRotation(), SpawnLocation
	);
	//데미지 숫자 Actor 생성
	ADamageNumberActor* DamageNumberActor =
		GetWorld()->SpawnActorDeferred<ADamageNumberActor>(
			DamageNumberClass, 
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (!DamageNumberActor)
	{
		return;
	}
	//실제로 받은 데미지를 전달
	DamageNumberActor->SetDamage(Damage);
	//데미지 숫자 Actor 생성 완료
	UGameplayStatics::FinishSpawningActor(
		DamageNumberActor,
		SpawnTransform
	);
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
	CurrentHealth = 0.0f;

	//현재 게임모드를 가져옴
	ALastCureGameMode* GM = Cast<ALastCureGameMode>(
		UGameplayStatics::GetGameMode(this)
	);
	if (GM)
	{
		//현재 좀비가 죽었다고 게임모드에 알림
		GM->HandleZombieDeath(this);
	}
	//혈청 드랍
	if (SerumClass)
	{
		ASerum* DroppedSerum =
			GetWorld()->SpawnActor<ASerum>(
				SerumClass,
				GetActorLocation(),
				FRotator::ZeroRotator
			);

		if (DroppedSerum)
		{
			//좀비의 혈청 보상량 전달
			DroppedSerum->SetSerumAmount(SerumReward);
		}
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
	OnDied.Broadcast(this); // 사망 이벤트 유지
}

void AEnemy::RemoveDeadBody()
{
	Destroy();
}

