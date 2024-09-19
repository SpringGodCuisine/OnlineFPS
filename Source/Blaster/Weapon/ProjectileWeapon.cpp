// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// 如果当前没有权限（服务器端），则直接返回
	if (!HasAuthority())return; 

	// 获取武器的所有者并将其转换为 APawn 类型
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	// 如果成功获取到 MuzzleFlashSocket
	if (MuzzleFlashSocket)
	{
		// 获取插槽的变换信息（位置、旋转、缩放）
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		// 计算从插槽位置到命中目标的方向向量
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 将方向向量转换为旋转角度，用于发射投射物
		FRotator TargetRotation = ToTarget.Rotation();
		// 如果投射物类存在并且武器的拥有者是 APawn 类型
		if (ProjectileClass && InstigatorPawn)
		{
			// 设置投射物的生成参数，设置所有者和发起者
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();		// 投射物的所有者
			SpawnParams.Instigator = InstigatorPawn;	// 发起者（Instigator）
			UWorld* World = GetWorld();
			if (World)
			{
				// 在世界中生成投射物实例
				World->SpawnActor<AProjectile>(
					ProjectileClass,	// 投射物的类
					SocketTransform.GetLocation(),	// 投射物的生成位置（即插槽位置）
					TargetRotation,		// 投射物的旋转角度（朝向命中目标）
					SpawnParams			// 生成参数（所有者和发起者）
				);
			}
		}
	}
}
