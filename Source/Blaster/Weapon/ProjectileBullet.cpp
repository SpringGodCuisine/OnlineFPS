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
	// 将当前子弹的拥有者转换为 ACharacter 类型
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		// 获取拥有者角色的控制器
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			// 对命中的目标应用伤害，参数包括：
			// - 目标 OtherActor
			// - 伤害值 Damage
			// - 拥有者的控制器 OwnerController
			// - 造成伤害的子弹对象 this
			// - 使用的伤害类型为默认的 UDamageType
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	// 调用父类的 OnHit 函数，确保父类的命中处理逻辑被执行
	Super::OnHit(HitComp, OtherActor,  OtherComp,  NormalImpulse, Hit);
}
