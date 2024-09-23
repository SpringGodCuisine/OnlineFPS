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

	//ProjectileMovementComponent ��ר��ΪͶ������Ƶ������ͨ�������ӵ��������������ƶ��͹켣����
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	//������ϵͳ�Ķ���������ļ����洢����ϵͳ����Դ�����á�
	UPROPERTY(EditAnyWhere)
	class UParticleSystem* Tracer;
	//��һ����������ڽ�����ϵͳʵ��������ʾ�ڳ����У���������ϵͳ�Ĳ�����ֹͣ��
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnyWhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnyWhere)
	class USoundCue* ImpactSound;
};
