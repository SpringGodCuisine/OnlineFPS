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

	// 如果 BlasterCharacter 尚未初始化，则尝试从 Pawn 获取 BlasterCharacter
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	// 如果 BlasterCharacter 仍然为空，直接返回
	if (BlasterCharacter == nullptr)return;

	// 获取角色的水平速度（忽略 Z 轴）
	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// 判断角色是否处于空中
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	// 判断角色是否在加速
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	// 检查角色是否装备武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	// 获取角色当前装备的武器
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	// 获取角色是否处于蹲下状态
	bIsCrouched = BlasterCharacter->bIsCrouched;
	// 判断角色是否在瞄准
	bAiming = BlasterCharacter->IsAiming();
	// 获取角色的转身状态
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	// 获取根骨骼旋转
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	// 是否淘汰
	bElimmed = BlasterCharacter->IsElimmed();

	// Offset Yaw for Strafing
	// 计算角色的偏航角度偏移，用于 Strafing 动作（侧移）
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.f);
	YawOffset = DeltaRotation.Yaw;

	// 检查是否在本地控制和是否具有权限（可用于调试）
	//if (!BlasterCharacter->HasAuthority() && BlasterCharacter->IsLocallyControlled())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw %f"), AimRotation.Yaw);
	//	UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f"), MovementRotation.Yaw);
	//}

	// 计算角色的 Lean 动作（左右倾斜）并进行平滑过渡
	CharacterRotationLastFrame = CharacterRotation;	// 保存上一帧的角色旋转
	CharacterRotation = BlasterCharacter->GetActorRotation();  // 获取当前帧的角色旋转
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;	// 计算目标 Lean 值
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);	// 平滑过渡 Lean 值
	Lean = FMath::Clamp(Interp, -90.f, 90.f); // 限制 Lean 值在 -90 到 90 之间

	// 计算角色的 Aim Offset（瞄准偏移）并进行平滑过渡
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	// 计算角色的左手位置
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		// 获取武器的 "LeftHandSocket" 插槽的世界变换
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		// 通过角色的骨骼转换工具，将世界位置转换到右手的骨骼空间
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);\
		// 更新左手位置和旋转
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// 如果角色是本地控制的（即当前玩家控制的角色）
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true; // 设置本地控制标志
			// 获取武器的 "Hand_R" 插槽的世界变换
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			// 计算右手的旋转，瞄准角色的击中目标
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
