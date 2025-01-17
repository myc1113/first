#include "TerminatorCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Terminator/Weapon/Weapon.h"
#include "Terminator/TerminatorComponents/CombatComponent.h"
#include "Terminator/TerminatorComponents/BuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TerminatorAnimInstance.h"
#include "Terminator/Terminator.h"
#include "Terminator/PlayerController/TerminatorPlayerController.h"
#include "Terminator/GameMode/TerminatorGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Terminator/PlayerState/TerminatorPlayerState.h"
#include "Terminator/Weapon/WeaponTypes.h"
#include "Terminator/PlayerStart/TeamPlayerStart.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Terminator/Weapon/Magazine.h"
#include "Terminator/PlayerController/EnemyAIController.h"

ATerminatorCharacter::ATerminatorCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->bUsePawnControlRotation = true;

	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSMesh"));
	FPSMesh->SetupAttachment(CameraComponent);
	FPSMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	FPSMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	FPSMesh->SetVisibility(false);
	FPSMesh->SetCastShadow(false);

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetMesh()->SetCustomDepthStencilValue(OUTLINE_BLUE);
	GetMesh()->MarkRenderStateDirty();
	EnableCustomDepth(false);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ATerminatorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATerminatorCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ATerminatorCharacter, Health);
	DOREPLIFETIME(ATerminatorCharacter, bDisableGameplay);
	DOREPLIFETIME(ATerminatorCharacter, Shield);
	DOREPLIFETIME(ATerminatorCharacter, bHasShield);
}

// Server Only (called by GameMode)
void ATerminatorCharacter::Elim(bool bPlayerLeftGame)
{
	if (Combat && Combat->EquippedWeapon && !Combat->EquippedWeapon->bDestroyDefaultWeapon && !bPlayerLeftGame)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim(bPlayerLeftGame);
}

void ATerminatorCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (TerminatorPlayerController)
	{
		TerminatorPlayerController->SetHUDWeaponAmmo(0);
		TerminatorPlayerController->SetHUDCarriedAmmo(0);
	}
	if (Combat && Combat->EquippedWeapon)
	{
			Combat->EquippedWeapon->GetMagazineActor()->Destroy();
			Combat->EquippedWeapon->Destroy();
	}
	bElimmed = true;
	PlayElimMontage();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetFPSMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetFPSMesh()->SetVisibility(false);

	if (ElimPoliceSound && !bPlayerLeftGame)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimPoliceSound, GetActorLocation());
	}

	if (bLeftGame)
	{
		GetWorldTimerManager().SetTimer(ElimTimer, this, &ATerminatorCharacter::ElimTimerFinished, LeftGameDelay);
	}
	else
	{
		GetWorldTimerManager().SetTimer(ElimTimer, this, &ATerminatorCharacter::ElimTimerFinished, ElimDelay);
	}
}

void ATerminatorCharacter::ElimTimerFinished()
{
	TerminatorGameMode = TerminatorGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATerminatorGameMode>() : TerminatorGameMode;
	if (TerminatorGameMode && !bLeftGame)
	{
		// Server Only
		TerminatorGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ATerminatorCharacter::ServerLeaveGame_Implementation()
{
	TerminatorGameMode = TerminatorGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATerminatorGameMode>() : TerminatorGameMode;
	TerminatorPlayerState = TerminatorPlayerState == nullptr ? GetPlayerState<ATerminatorPlayerState>() : TerminatorPlayerState;
	if (TerminatorGameMode && TerminatorPlayerState)
	{
		TerminatorGameMode->PlayerLeftGame(TerminatorPlayerState);
	}
}

void ATerminatorCharacter::SetBlueTeamMaterial()
{
	GetMesh()->SetMaterial(0, BlueJacketMaterial);
	GetMesh()->SetMaterial(1, BluePantsMaterial);
	GetMesh()->SetMaterial(2, BlueGloveMaterial);
	GetMesh()->SetMaterial(3, BlueHelmetMaterial);
	GetMesh()->SetMaterial(4, BlueEyeMaterial);
	GetMesh()->SetMaterial(5, BlueFaceMaterial);
	GetMesh()->SetMaterial(6, BlueGasmaskMaterial);
	GetMesh()->SetMaterial(7, BlueEquipmentMaterial);
}

void ATerminatorCharacter::SetRedTeamMaterial()
{
	GetMesh()->SetMaterial(0, RedJacketMaterial);
	GetMesh()->SetMaterial(1, RedPantsMaterial);
	GetMesh()->SetMaterial(2, RedGloveMaterial);
	GetMesh()->SetMaterial(3, RedHelmetMaterial);
	GetMesh()->SetMaterial(4, RedEyeMaterial);
	GetMesh()->SetMaterial(5, RedFaceMaterial);
	GetMesh()->SetMaterial(6, RedGasmaskMaterial);
	GetMesh()->SetMaterial(7, RedEquipmentMaterial);
}

void ATerminatorCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr) return;

	switch (Team)
	{
	case ETeam::ET_NoTeam:
		SetBlueTeamMaterial();
		break;
	case ETeam::ET_BlueTeam:
		SetBlueTeamMaterial();
		break;
	case ETeam::ET_RedTeam:
		SetRedTeamMaterial();
		break;
	}
}

void ATerminatorCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ATerminatorCharacter::ReceiveDamage);
	}
}

void ATerminatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	PollInit();
}

void ATerminatorCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	AimOffset(DeltaTime);
}

void ATerminatorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATerminatorCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATerminatorCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATerminatorCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ATerminatorCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ATerminatorCharacter::LookUp);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATerminatorCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATerminatorCharacter::CrouchButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATerminatorCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATerminatorCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATerminatorCharacter::ReloadButtonPressed);
}

void ATerminatorCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
}

// Server and All Clients
void ATerminatorCharacter::PlayFireMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = FName("RifleAim");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATerminatorCharacter::PlayFPSFireMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetFPSMesh()->GetAnimInstance();
	if (AnimInstance && FPSFireWeaponMontage)
	{
		AnimInstance->Montage_Play(FPSFireWeaponMontage);
		FName SectionName = FName("FPSRifle");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATerminatorCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimInstance* FPSAnimInstance = GetFPSMesh()->GetAnimInstance();

	FName SectionName;
	switch (Combat->EquippedWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_AssualtRifle:
		SectionName = FName("Rifle");
		break;
	}

	if (ReloadMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	if (IsLocallyControlled() && FPSReloadMontage && FPSAnimInstance)
	{
		FPSAnimInstance->Montage_Play(FPSReloadMontage);
		FPSAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATerminatorCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ATerminatorCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// Server Only
void ATerminatorCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	TerminatorGameMode = TerminatorGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATerminatorGameMode>() : TerminatorGameMode;
	if (bElimmed || TerminatorGameMode == nullptr) return;
	Damage = TerminatorGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);  // Replicated Variable (ReplicatedUsing = OnRep_Health)
	UpdateHUDHealth();
	if (Damage > 0.f)
	{
		PlayHitReactMontage();
	}

	if (Health == 0.f)
	{
		if (TerminatorGameMode)
		{
			TerminatorPlayerController = TerminatorPlayerController == nullptr ? Cast<ATerminatorPlayerController>(Controller) : TerminatorPlayerController;
			if (InstigatorController->IsA(AEnemyAIController::StaticClass()))
			{
				TerminatorGameMode->PlayerEliminatedByEnemy(this, TerminatorPlayerController);
			}
			else
			{
				ATerminatorPlayerController* AttackController = Cast<ATerminatorPlayerController>(InstigatorController);
				TerminatorGameMode->PlayerEliminated(this, TerminatorPlayerController, AttackController);
			}
		}
	}
}

void ATerminatorCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ATerminatorCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ATerminatorCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATerminatorCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATerminatorCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	Crouch();
}

void ATerminatorCharacter::CrouchButtonReleased()
{
	if (bDisableGameplay) return;
	UnCrouch();
}

void ATerminatorCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

float ATerminatorCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ATerminatorCharacter::AimOffset(float DeltaTime)
{
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)  // Standing Still and Not Jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)    // Running or Jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ATerminatorCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// [270, 360) -> [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ATerminatorCharacter::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ATerminatorCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ATerminatorCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ATerminatorCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

// Client Only
void ATerminatorCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ATerminatorCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	//if (Shield < LastShield)
	//{
	//	PlayHitReactMontage();
	//}
}

void ATerminatorCharacter::UpdateHUDHealth()
{
	TerminatorPlayerController = TerminatorPlayerController == nullptr ? Cast<ATerminatorPlayerController>(Controller) : TerminatorPlayerController;
	if (TerminatorPlayerController)
	{
		TerminatorPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ATerminatorCharacter::UpdateHUDShield()
{
	TerminatorPlayerController = TerminatorPlayerController == nullptr ? Cast<ATerminatorPlayerController>(Controller) : TerminatorPlayerController;
	if (TerminatorPlayerController)
	{
		TerminatorPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ATerminatorCharacter::UpdateHUDAmmo()
{
	TerminatorPlayerController = TerminatorPlayerController == nullptr ? Cast<ATerminatorPlayerController>(Controller) : TerminatorPlayerController;
	if (TerminatorPlayerController && Combat && Combat->EquippedWeapon)
	{
		TerminatorPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
		TerminatorPlayerController->SetHUDCarriedAmmo(Combat->EquippedWeapon->GetCarriedAmmo());
	}
}

void ATerminatorCharacter::SpawnDefaultWeapon()
{
	TerminatorGameMode = TerminatorGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATerminatorGameMode>() : TerminatorGameMode;
	UWorld* World = GetWorld();
	if (TerminatorGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		// Server Only
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyDefaultWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
			EquipState = EEquipState::EES_EquipMainWeapon;
		}
	}
}

void ATerminatorCharacter::SetSpawnPoint()
{
	if (HasAuthority() && TerminatorPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamPlayerStart = Cast<ATeamPlayerStart>(Start);
			if (TeamPlayerStart && TeamPlayerStart->Team == TerminatorPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamPlayerStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void ATerminatorCharacter::OnPlayerStateInitialized()
{
	TerminatorPlayerState->AddToScore(0.f);
	TerminatorPlayerState->AddToDefeats(0);

	SetTeamColor(TerminatorPlayerState->GetTeam());
	SetSpawnPoint();
}

void ATerminatorCharacter::PollInit()
{
	if (TerminatorPlayerState == nullptr)
	{
		TerminatorPlayerState = GetPlayerState<ATerminatorPlayerState>();
		if (TerminatorPlayerState)
		{
			OnPlayerStateInitialized();
		}
	}
	
	if (TerminatorPlayerController == nullptr)
	{
		TerminatorPlayerController = TerminatorPlayerController == nullptr ? Cast<ATerminatorPlayerController>(Controller) : TerminatorPlayerController;
		if (TerminatorPlayerController)
		{
			SpawnDefaultWeapon();
			UpdateHUDAmmo();
			UpdateHUDHealth();

			if (IsLocallyControlled())
			{
				FPSMesh->SetVisibility(true);
				GetMesh()->SetVisibility(false);
			}
		}
	}
}

// Server Only
void ATerminatorCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (EquipState == EEquipState::EES_EquipMainWeapon || EquipState == EEquipState::EES_EquipAll) { return; }
	OverlappingWeapon = Weapon;
	if (OverlappingWeapon && Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
		if (EquipState == EEquipState::EES_Initial)
		{
			EquipState = EEquipState::EES_EquipMainWeapon;
		}
		else
		{
			EquipState = EEquipState::EES_EquipAll;
		}
	}
}

AWeapon* ATerminatorCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) { return nullptr; }
	return Combat->EquippedWeapon;
}

FVector ATerminatorCharacter::GetHitTarget()
{
	if (Combat == nullptr) { return FVector(); }
	return Combat->HitTarget;
}

ECombatState ATerminatorCharacter::GetCombatState() const
{
	if (Combat == nullptr) { return ECombatState::ECS_MAX; }
	return Combat->CombatState;
}

void ATerminatorCharacter::EnableCustomDepth(bool bEnable)
{
	if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(bEnable);
	}
}
