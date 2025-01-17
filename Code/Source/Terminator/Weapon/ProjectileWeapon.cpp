// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

// Server and All Clents by Multicast
// AProjectileWeapon是Weapon的子类 Fire是虚函数
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	// 先执行Weapon::Fire(HitTarget)
	Super::Fire(HitTarget);

	if (!HasAuthority()) { return; }

	// Server Only
	// Spawn Projectile
	// AProjectile is Replicated
	APawn* InvestigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector MuzzleToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = MuzzleToTarget.Rotation();
		if (ProjectileClass && InvestigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InvestigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			}
		}
	}
}
