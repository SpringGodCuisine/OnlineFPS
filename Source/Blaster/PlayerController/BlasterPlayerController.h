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
	// OnPossess �� Unreal Engine �е�һ���麯������������ռ�У�Possess��һ�� Pawn����Ϸ��ɫ��ʱ�ᱻ���á������ OnPossess ���ܴ���һЩ�����߼������罫���������ɫ����������
	// OnPossess �����ĵ���˳���� Pawn �� BeginPlay ����֮��Ҳ����˵����һ�� Pawn ����ҿ�����ռ��ʱ��BeginPlay ������ OnPossess ������
	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;

private:
	class ABlasterHUD* BlasterHUD;
};
