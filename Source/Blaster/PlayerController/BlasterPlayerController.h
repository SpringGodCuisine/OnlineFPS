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

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/**
	* Sync time between client and server
	*/
	
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequset);

	float ClientServerDelta = 0.f; // difference between client and server time;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f; // how often to sync time with server

	float TimeSyncRunningTime = 0.f; // how long we've been running the time sync
	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt;
};
