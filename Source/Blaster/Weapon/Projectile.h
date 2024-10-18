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
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	// 只在服务器上获得OnHit事件
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnyWhere)
	float Damage = 20.f;

	UPROPERTY(EditAnyWhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnyWhere)
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	//ProjectileMovementComponent 是专门为投射物设计的组件，通常用于子弹、火箭等物体的移动和轨迹计算
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

private:

	//是粒子系统的定义和配置文件，存储粒子系统的资源和设置。
	UPROPERTY(EditAnyWhere)
	class UParticleSystem* Tracer;
	//是一个组件，用于将粒子系统实例化并显示在场景中，控制粒子系统的播放与停止。
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
