#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Terminator/TerminatorTypes/TurningInPlace.h"
#include "Terminator/TerminatorTypes/CombatState.h"
#include "Terminator/TerminatorTypes/Team.h"
#include "TerminatorCharacter.generated.h"

UENUM(BlueprintType)
enum class EEquipState : uint8
{
	EES_Initial UMETA(DisplayName = "Initial State"),
	EES_EquipSecondaryWeapon UMETA(DisplayName = "Equipped"),
	EES_EquipMainWeapon UMETA(DisplayName = "EquipMainWeapon"),
	EES_EquipAll UMETA(DisplayName = "EquipAll"),

	EES_MAX UMETA(DisplayName = "DefaultMAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class TERMINATOR_API ATerminatorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATerminatorCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage();
	void PlayFPSFireMontage();
	void PlayReloadMontage();
	void PlayElimMontage();
	void Elim(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;
	bool bLeftGame = false;

	void SetBlueTeamMaterial();
	void SetRedTeamMaterial();
	void SetTeamColor(ETeam Team);

	void EnableCustomDepth(bool bEnable);

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void CrouchButtonPressed();
	void CrouchButtonReleased();
	void ReloadButtonPressed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	// Poll for any relevant classed and initialize our HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* FPSMesh;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(Replicated)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere, Category = "Equip State")
	EEquipState EquipState;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FPSFireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FPSReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	bool bRotateRootBone;
	float CalculateSpeed();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 5000.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 5000.f;

	UPROPERTY(Replicated, VisibleAnywhere)
	bool bHasShield = false;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UPROPERTY()
	class ATerminatorPlayerController* TerminatorPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	float LeftGameDelay = 0.01f;

	void ElimTimerFinished();

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimPoliceSound;

	UPROPERTY()
	class ATerminatorPlayerState* TerminatorPlayerState;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueJacketMaterial;
	
	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BluePantsMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueGloveMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueHelmetMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueEyeMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueFaceMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueGasmaskMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* BlueEquipmentMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedJacketMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedPantsMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedGloveMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedHelmetMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedEyeMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedFaceMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedGasmaskMaterial;

	UPROPERTY(EditAnywhere, Category = Materials)
	class UMaterialInstance* RedEquipmentMaterial;

	UPROPERTY()
	class ATerminatorGameMode* TerminatorGameMode;


public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return Buff; }
	//bool IsWeaponEquipped();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget();
	FORCEINLINE UCameraComponent* GetCamera() const { return CameraComponent; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE USkeletalMeshComponent* GetFPSMesh() const { return FPSMesh; }
};
