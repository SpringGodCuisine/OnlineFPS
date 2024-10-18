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

	// ֻ�ڷ������ϻ��OnHit�¼�
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

	//ProjectileMovementComponent ��ר��ΪͶ������Ƶ������ͨ�������ӵ��������������ƶ��͹켣����
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

	//������ϵͳ�Ķ���������ļ����洢����ϵͳ����Դ�����á�
	UPROPERTY(EditAnyWhere)
	class UParticleSystem* Tracer;
	//��һ����������ڽ�����ϵͳʵ��������ʾ�ڳ����У���������ϵͳ�Ĳ�����ֹͣ��
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
