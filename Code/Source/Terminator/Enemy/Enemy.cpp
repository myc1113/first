#include "Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Terminator/GameMode/TerminatorGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->MaxWalkSpeed = 112.5f;
}

void AEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemy, PlayMontageInfo);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AEnemy::ReceiveDamage);
	}

	EnemyController = Cast<AAIController>(GetController());

	if (PatrolTargets.IsValidIndex(CurrentPatrolIndex) && EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(PatrolTargets[CurrentPatrolIndex]);
		//MoveRequest.SetAcceptanceRadius(15.f);
		EnemyController->MoveTo(MoveRequest);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Server Only
void AEnemy::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	ATerminatorGameMode* TerminatorGameMode = GetWorld()->GetAuthGameMode<ATerminatorGameMode>();
	if (TerminatorGameMode == nullptr) return;
	Damage = TerminatorGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (Health == 0.f)
	{
		if (TerminatorGameMode)
		{
			ATerminatorPlayerController* AttackController = Cast<ATerminatorPlayerController>(InstigatorController);
			TerminatorGameMode->MutantEliminated(AttackController);
		}
	}
}

// Play Montage, Spawn Emiitter, Set Collision if death
void AEnemy::OnRep_PlayMontageInfo()
{
	PlayMontage();
}

void AEnemy::PlayMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && PlayMontageInfo.Montage)
	{
		AnimInstance->Montage_Play(PlayMontageInfo.Montage);
		AnimInstance->Montage_JumpToSection(PlayMontageInfo.StartSectionName, PlayMontageInfo.Montage);
	}

	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, PlayMontageInfo.ImpactPoint);
	}

	if (PlayMontageInfo.bDeathAnimation)
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Server Only
void AEnemy::GetHit(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();	// Normalized already
	const FVector ImpactPointChangeZ(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactPointChangeZ - GetActorLocation()).GetSafeNormal();

	// 向量点乘判断前后
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	// 向量叉乘判断左右 UE是左手系
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName SectionName("FromBack");
	if (Theta >= -45.f && Theta < 45.f)
	{
		SectionName = FName("FromFront");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		SectionName = FName("FromRight");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		SectionName = FName("FromLeft");
	}

	if (Health > 0.f)
	{
		// 考虑多方向播放受击Montage
		// PlayMontageInfo is a replicated struct
		PlayMontageInfo.Montage = HitReactMontage;
		PlayMontageInfo.StartSectionName = SectionName;
		if (auto GameState = UGameplayStatics::GetGameState(this))
		{
			// 确保PlayMontageInfo发生变化 确保Client也执行
			PlayMontageInfo.TimeRequested = GameState->GetServerWorldTimeSeconds();
		}
		PlayMontageInfo.ImpactPoint = ImpactPoint;
		PlayMontageInfo.bDeathAnimation = false;

		PlayMontage();
	}
	else
	{
		// 考虑多方向播放死亡Montage
		// PlayMontageInfo is a replicated struct
		PlayMontageInfo.Montage = DeathMontage;
		PlayMontageInfo.StartSectionName = SectionName;
		if (auto GameState = UGameplayStatics::GetGameState(this))
		{
			// 确保PlayMontageInfo发生变化 确保Client也执行
			PlayMontageInfo.TimeRequested = GameState->GetServerWorldTimeSeconds();
		}
		PlayMontageInfo.ImpactPoint = ImpactPoint;
		PlayMontageInfo.bDeathAnimation = true;

		PlayMontage();

		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();

		SetLifeSpan(5.f);
	}
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

//void AEnemy::Patrol()
//{
//	if (bNeedRest)
//	{
//		// 休息时间不够 继续休息 直接Return
//		if (GetWorld()->GetTimeSeconds() - RestStartTime < RestTime) return;
//		// 休息时间到了 开始巡逻
//		bNeedRest = false;
//		PatrolStartTime = GetWorld()->GetTimeSeconds();
//	}
//
//	// 如果到了某个巡逻点 就准备去往下一个
//	if (InTargetRange(PatrolTargets[CurrentPatrolIndex], PatrolRadius))
//	{
//		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolTargets.Num();
//	}
//
//	// 不在观察且不在休息
//	if (EnemyController)
//	{
//		FAIMoveRequest MoveRequest;
//		MoveRequest.SetGoalActor(PatrolTargets[CurrentPatrolIndex]);
//		EnemyController->MoveTo(MoveRequest);
//	}
//
//	// 累了 开始休息
//	if (GetWorld()->GetTimeSeconds() - PatrolStartTime >= PatrolTime)
//	{
//		EnemyController->StopMovement();
//		bNeedRest = true;
//		RestStartTime = GetWorld()->GetTimeSeconds();
//	}
//}

void AEnemy::Patrol()
{
	// 如果到了某个巡逻点 就准备去往下一个
	if (InTargetRange(PatrolTargets[CurrentPatrolIndex], PatrolRadius))
	{
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolTargets.Num();
	}

	if (EnemyController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(PatrolTargets[CurrentPatrolIndex]);
		EnemyController->MoveTo(MoveRequest);
	}
}

void AEnemy::Attack(AActor* InTargetActor)
{
	TargetActor = InTargetActor;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		const float Selection = FMath::RandRange(0.f, 1.f);
		if (Selection <= 0.4f)
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack1"), AttackMontage);
		}
		else if(Selection <= 0.8f)
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack2"), AttackMontage);
		}
		else if (Selection <= 0.9f)
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack3"), AttackMontage);
		}
		else
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack4"), AttackMontage);
		}
	}
}

void AEnemy::HittingTargetActor()
{
	UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, Controller, this, UDamageType::StaticClass());

	if (AttackSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, AttackSound, GetActorLocation());
	}
}
