// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);

	// OnPossess �� Unreal Engine �е�һ���麯������������ռ�У�Possess��һ�� Pawn����Ϸ��ɫ��ʱ�ᱻ���á������ OnPossess ���ܴ���һЩ�����߼������罫���������ɫ����������
	// OnPossess �����ĵ���˳���� Pawn �� BeginPlay ����֮��Ҳ����˵����һ�� Pawn ����ҿ�����ռ��ʱ��BeginPlay ������ OnPossess ������
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt;
};
