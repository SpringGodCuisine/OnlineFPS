// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// ����ǰ�ӵ���ӵ����ת��Ϊ ACharacter ����
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		// ��ȡӵ���߽�ɫ�Ŀ�����
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			// �����е�Ŀ��Ӧ���˺�������������
			// - Ŀ�� OtherActor
			// - �˺�ֵ Damage
			// - ӵ���ߵĿ����� OwnerController
			// - ����˺����ӵ����� this
			// - ʹ�õ��˺�����ΪĬ�ϵ� UDamageType
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	// ���ø���� OnHit ������ȷ����������д����߼���ִ��
	Super::OnHit(HitComp, OtherActor,  OtherComp,  NormalImpulse, Hit);
}
