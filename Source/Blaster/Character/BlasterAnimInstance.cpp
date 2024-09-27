// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// ��� BlasterCharacter ��δ��ʼ�������Դ� Pawn ��ȡ BlasterCharacter
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	// ��� BlasterCharacter ��ȻΪ�գ�ֱ�ӷ���
	if (BlasterCharacter == nullptr)return;

	// ��ȡ��ɫ��ˮƽ�ٶȣ����� Z �ᣩ
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// �жϽ�ɫ�Ƿ��ڿ���
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	// �жϽ�ɫ�Ƿ��ڼ���
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	// ����ɫ�Ƿ�װ������
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	// ��ȡ��ɫ��ǰװ��������
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	// ��ȡ��ɫ�Ƿ��ڶ���״̬
	bIsCrouched = BlasterCharacter->bIsCrouched;
	// �жϽ�ɫ�Ƿ�����׼
	bAiming = BlasterCharacter->IsAiming();
	// ��ȡ��ɫ��ת��״̬
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	// ��ȡ��������ת
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	// �Ƿ���̭
	bElimmed = BlasterCharacter->IsElimmed();

	// Offset Yaw for Strafing
	// �����ɫ��ƫ���Ƕ�ƫ�ƣ����� Strafing ���������ƣ�
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.f);
	YawOffset = DeltaRotation.Yaw;

	// ����Ƿ��ڱ��ؿ��ƺ��Ƿ����Ȩ�ޣ������ڵ��ԣ�
	//if (!BlasterCharacter->HasAuthority() && BlasterCharacter->IsLocallyControlled())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f"), AimRotation.Yaw);
	//	UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f"), MovementRotation.Yaw);
	//}

	// �����ɫ�� Lean ������������б��������ƽ������
	CharacterRotationLastFrame = CharacterRotation;	// ������һ֡�Ľ�ɫ��ת
	CharacterRotation = BlasterCharacter->GetActorRotation();  // ��ȡ��ǰ֡�Ľ�ɫ��ת
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;	// ����Ŀ�� Lean ֵ
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);	// ƽ������ Lean ֵ
	Lean = FMath::Clamp(Interp, -90.f, 90.f); // ���� Lean ֵ�� -90 �� 90 ֮��

	// �����ɫ�� Aim Offset����׼ƫ�ƣ�������ƽ������
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// �����ɫ������λ��
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// ��ȡ������ "LeftHandSocket" ��۵�����任
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		// ͨ����ɫ�Ĺ���ת�����ߣ�������λ��ת�������ֵĹ����ռ�
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);\
		// ��������λ�ú���ת
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// �����ɫ�Ǳ��ؿ��Ƶģ�����ǰ��ҿ��ƵĽ�ɫ��
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true; // ���ñ��ؿ��Ʊ�־
			// ��ȡ������ "Hand_R" ��۵�����任
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			// �������ֵ���ת����׼��ɫ�Ļ���Ŀ��
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}

		//UE_LOG(LogTemp, Warning, TEXT("HitTarget::%s"), *BlasterCharacter->GetHitTarget().ToString());
		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);

		bUseFABRIK = BlasterCharacter->GetComnbatState() != ECombatState::ECS_Reloading;
	}
}
