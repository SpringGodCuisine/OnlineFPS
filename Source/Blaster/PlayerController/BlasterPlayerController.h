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

	// OnPossess 是 Unreal Engine 中的一个虚函数，当控制器占有（Possess）一个 Pawn（游戏角色）时会被调用。父类的 OnPossess 可能处理一些基础逻辑，比如将控制器与角色关联起来。
	// OnPossess 函数的调用顺序在 Pawn 的 BeginPlay 函数之后。也就是说，当一个 Pawn 被玩家控制器占有时，BeginPlay 会先于 OnPossess 触发。
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
