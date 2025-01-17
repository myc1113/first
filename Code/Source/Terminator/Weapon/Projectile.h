#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class TERMINATOR_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 30.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 100.f;

	UPROPERTY(EditAnywhere)
	float EnemyDamage = 35.f;

	UPROPERTY(EditAnywhere)
	float EnemyHeadShotDamage = 35.f;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* ImpactEffect;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

public:	
};
