#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Terminator/Interfaces/HitInterface.h"
#include "Enemy.generated.h"

USTRUCT(BlueprintType)
struct FCharacterPlayMontageInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FName StartSectionName = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	double TimeRequested = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool RequestStop = false;

	// For Spawn Emitter
	UPROPERTY(BlueprintReadWrite)
	FVector ImpactPoint = FVector::ZeroVector;

	bool bDeathAnimation = false;
};

UCLASS()
class TERMINATOR_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void GetHit(const FVector& ImpactPoint) override;

	UFUNCTION(BlueprintCallable)
	void Patrol();

	UFUNCTION(BlueprintCallable)
	void Attack(AActor* InTargetActor);

	UFUNCTION(BlueprintCallable)
	void HittingTargetActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_PlayMontageInfo)
	FCharacterPlayMontageInfo PlayMontageInfo;

	UFUNCTION()
	void OnRep_PlayMontageInfo();

	void PlayMontage();

	bool InTargetRange(AActor* Target, double Radius);

private:
	float MaxHealth = 100.f;
	float Health = 100.f;

	bool bDeath = false;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	class USoundCue* AttackSound;

	UPROPERTY(EditAnywhere)
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	int32 CurrentPatrolIndex = 0;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 100.f;

	UPROPERTY()
	AActor* TargetActor;

public:	

};
