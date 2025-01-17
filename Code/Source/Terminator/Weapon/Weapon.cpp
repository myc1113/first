#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Terminator/Character/TerminatorCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "Magazine.h"
#include "Silencer.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(OUTLINE_TAN);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(false);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	}

	if (MagazineClass)
	{
		const USkeletalMeshSocket* MagazineAttachSocket = WeaponMesh->GetSocketByName(FName("MagazineAttach"));
		UWorld* World = GetWorld();
		if (MagazineAttachSocket && World)
		{
			MagazineActor = MagazineActor == nullptr ? World->SpawnActor<AMagazine>(MagazineClass) : MagazineActor;
			MagazineAttachSocket->AttachActor(MagazineActor, WeaponMesh);
		}
	}

	if (SilencerClass)
	{
		const USkeletalMeshSocket* SilencerSocket = WeaponMesh->GetSocketByName(FName("SilencerSocket"));
		UWorld* World = GetWorld();
		if (SilencerSocket && World)
		{
			ASilencer* SilencerActor = World->SpawnActor<ASilencer>(SilencerClass);
			SilencerSocket->AttachActor(SilencerActor, WeaponMesh);
		}
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
	DOREPLIFETIME(AWeapon, CarriedAmmo);
}

// Server and All Clents by Multicast
void AWeapon::Fire(const FVector& HitTarget)
{
	// ²¥·ÅÒ»´ÎÇ¹Ë¨Ìø¶¯¶¯»­
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	// Ç¹¿Ú»ðÑæ
	if (MuzzleFlashEffect)
	{
		FTransform MuzzleFlashTransform = WeaponMesh->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlashEffect, MuzzleFlashTransform.GetLocation(), MuzzleFlashTransform.GetRotation().Rotator());
	}
	// µ¯¿Ç
	if (CasingClass)
	{
		const USkeletalMeshSocket* ShellEjectSocket = WeaponMesh->GetSocketByName(FName("ShellEject"));
		if (ShellEjectSocket)
		{
			FTransform SocketTransform = ShellEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}

	SpendRound();
}

// Locally Controlled Only
void AWeapon::PlayReloadAnimation()
{
	if (ReloadAnimation)
	{
		WeaponMesh->PlayAnimation(ReloadAnimation, false);
	}
}

// Server Only
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	TerminatorOwnerCharacter = nullptr;
	TerminatorOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::MinusCarriedAmmo(int32 CarriedAmmoToMinus)
{
	CarriedAmmo = FMath::Clamp(CarriedAmmo - CarriedAmmoToMinus, 0, CarriedAmmoCapacity);
	SetHUDCarriedAmmo();
}

// Server Only
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATerminatorCharacter* TerminatorCharacter = Cast<ATerminatorCharacter>(OtherActor);
	if (TerminatorCharacter)
	{
		TerminatorCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::SetHUDAmmo()
{
	TerminatorOwnerCharacter = TerminatorOwnerCharacter == nullptr ? Cast<ATerminatorCharacter>(GetOwner()) : TerminatorOwnerCharacter;
	if (TerminatorOwnerCharacter)
	{
		TerminatorOwnerController = TerminatorOwnerController == nullptr ? Cast<ATerminatorPlayerController>(TerminatorOwnerCharacter->Controller) : TerminatorOwnerController;
		if (TerminatorOwnerController)
		{
			TerminatorOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SetHUDCarriedAmmo()
{
	TerminatorOwnerCharacter = TerminatorOwnerCharacter == nullptr ? Cast<ATerminatorCharacter>(GetOwner()) : TerminatorOwnerCharacter;
	if (TerminatorOwnerCharacter)
	{
		TerminatorOwnerController = TerminatorOwnerController == nullptr ? Cast<ATerminatorPlayerController>(TerminatorOwnerCharacter->Controller) : TerminatorOwnerController;
		if (TerminatorOwnerController)
		{
			TerminatorOwnerController->SetHUDCarriedAmmo(CarriedAmmo);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_CarriedAmmo()
{
	SetHUDCarriedAmmo();
}

void AWeapon::FillAmmoAndCarriedAmmo()
{
	Ammo = MagCapacity;
	CarriedAmmo = CarriedAmmoCapacity;
	SetHUDAmmo();
	SetHUDCarriedAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		TerminatorOwnerCharacter = nullptr;
		TerminatorOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
		SetHUDCarriedAmmo();
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

bool AWeapon::IsEmpty() const
{
	return Ammo <= 0;
}

bool AWeapon::IsFull() const
{
	return Ammo == MagCapacity;
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

