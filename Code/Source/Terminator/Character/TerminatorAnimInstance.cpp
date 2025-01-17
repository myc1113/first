#include "TerminatorAnimInstance.h"
#include "TerminatorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Terminator/Weapon/Weapon.h"
#include "Terminator/TerminatorTypes/CombatState.h"

void UTerminatorAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	TerminatorCharacter = Cast<ATerminatorCharacter>(TryGetPawnOwner());
}

void UTerminatorAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (TerminatorCharacter == nullptr)
	{
		TerminatorCharacter = Cast<ATerminatorCharacter>(TryGetPawnOwner());
	}
	if (TerminatorCharacter == nullptr) { return; }

	FVector Velocity = TerminatorCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = TerminatorCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = TerminatorCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bIsCrouched = TerminatorCharacter->bIsCrouched;

	EquippedWeapon = TerminatorCharacter->GetEquippedWeapon();
	if (EquippedWeapon)
	{
		bEquippedWeapon = true;
	}
	else
	{
		bEquippedWeapon = false;
	}

	// 0 -> 180  flip to -180  -180 -> 0
	FRotator AimRotation = TerminatorCharacter->GetBaseAimRotation();
	FRotator MovementRoatation = UKismetMathLibrary::MakeRotFromX(TerminatorCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRoatation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.0f);
	YawOffset = DeltaRotation.Yaw;

	AO_Yaw = TerminatorCharacter->GetAO_Yaw();
	AO_Pitch = TerminatorCharacter->GetAO_Pitch();

	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TerminatorCharacter->GetMesh() && TerminatorCharacter->GetFPSMesh())
	{
		if (TerminatorCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + RightHandTransform.GetLocation() - TerminatorCharacter->GetHitTarget());
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
			// Code above don't need in fps game. RightHandRotation won't be used in animation blueprint.
			FPSLeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("FPSLeftHandSocket"), ERelativeTransformSpace::RTS_World);
			FVector OutPosition;
			FRotator OutRotation;
			TerminatorCharacter->GetFPSMesh()->TransformToBoneSpace(FName("hand_r"), FPSLeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
			FPSLeftHandTransform.SetLocation(OutPosition);
			FPSLeftHandTransform.SetRotation(FQuat(OutRotation));
		}
		else
		{
			LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
			FVector OutPosition;
			FRotator OutRotation;
			TerminatorCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
			LeftHandTransform.SetLocation(OutPosition);
			LeftHandTransform.SetRotation(FQuat(OutRotation));
		}
	}

	TurningInPlace = TerminatorCharacter->GetTurningInPlace();

	bRotateRootBone = TerminatorCharacter->ShouldRotateRootBone();

	bElimmed = TerminatorCharacter->IsElimmed();

	bUseFABRIK = true;
	if (TerminatorCharacter->GetCombatState() == ECombatState::ECS_Reloading)
	{
		bUseFABRIK = false;
	}

	bUseAimOffsets = true;
	if (TerminatorCharacter->GetCombatState() == ECombatState::ECS_Reloading || TerminatorCharacter->GetDisableGameplay())
	{
		bUseAimOffsets = false;
	}
}
