#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Terminator/DestructibleObjects/Cube.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Terminator/TerminatorComponents/CombatComponent.h"
#include "Terminator/Interfaces/HitInterface.h"
#include "Terminator/Enemy/Enemy.h"

// Server Only
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ATerminatorCharacter* TOwnerCharacter = Cast<ATerminatorCharacter>(OwnerCharacter);
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			float DamageToCause = 0.f;
			if (OtherActor->IsA(ATerminatorCharacter::StaticClass()))
			{
				DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
			}
			else if (OtherActor->IsA(AEnemy::StaticClass()))
			{
				DamageToCause = Hit.BoneName.ToString() == FString("head") ? EnemyHeadShotDamage : EnemyDamage;
			}
			else if (OtherActor->IsA(ACube::StaticClass()))
			{
				DamageToCause = 1.0f;
			}
			if (DamageToCause > 0.f)
			{
				if (TOwnerCharacter)
				{
					TOwnerCharacter->GetCombatComponent()->ActivateHitCrosshair();
				}
			}
			UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());

			IHitInterface* HitInterface = Cast<IHitInterface>(OtherActor);
			if (HitInterface)
			{
				HitInterface->GetHit(Hit.ImpactPoint);
			}
		}
	}

	// Destroy×Óµ¯
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
