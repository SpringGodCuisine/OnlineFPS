// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnyWhere)
	float Damage = 20.f;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	//ProjectileMovementComponent 是专门为投射物设计的组件，通常用于子弹、火箭等物体的移动和轨迹计算
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	//是粒子系统的定义和配置文件，存储粒子系统的资源和设置。
	UPROPERTY(EditAnyWhere)
	class UParticleSystem* Tracer;
	//是一个组件，用于将粒子系统实例化并显示在场景中，控制粒子系统的播放与停止。
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnyWhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnyWhere)
	class USoundCue* ImpactSound;
};
