// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// �����ǰû��Ȩ�ޣ��������ˣ�����ֱ�ӷ���
	if (!HasAuthority())return; 

	// ��ȡ�����������߲�����ת��Ϊ APawn ����
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	// ����ɹ���ȡ�� MuzzleFlashSocket
	if (MuzzleFlashSocket)
	{
		// ��ȡ��۵ı任��Ϣ��λ�á���ת�����ţ�
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		// ����Ӳ��λ�õ�����Ŀ��ķ�������
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// ����������ת��Ϊ��ת�Ƕȣ����ڷ���Ͷ����
		FRotator TargetRotation = ToTarget.Rotation();
		// ���Ͷ��������ڲ���������ӵ������ APawn ����
		if (ProjectileClass && InstigatorPawn)
		{
			// ����Ͷ��������ɲ��������������ߺͷ�����
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();		// Ͷ�����������
			SpawnParams.Instigator = InstigatorPawn;	// �����ߣ�Instigator��
			UWorld* World = GetWorld();
			if (World)
			{
				// ������������Ͷ����ʵ��
				World->SpawnActor<AProjectile>(
					ProjectileClass,	// Ͷ�������
					SocketTransform.GetLocation(),	// Ͷ���������λ�ã������λ�ã�
					TargetRotation,		// Ͷ�������ת�Ƕȣ���������Ŀ�꣩
					SpawnParams			// ���ɲ����������ߺͷ����ߣ�
				);
			}
		}
	}
}
