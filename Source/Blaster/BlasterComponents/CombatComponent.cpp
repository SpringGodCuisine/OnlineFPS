// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
} 

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed == true)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (bCanFire && EquippedWeapon)
	{
		bCanFire = false;
		ServerFire(HitTarget);
		CrosshairShootingFactor = 0.75f;
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr || Character == nullptr)return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
 
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)return;

	// ����ǰҪװ������������Ϊ EquippedWeapon
	EquippedWeapon = WeaponToEquip;
	// ����������״̬Ϊ "��װ��"��EWS_Equipped��
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// ��ȡ��ɫ���ֵĹ������ "RightHandSocket"
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		// ���������ӵ���ɫ�����ֲ��
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	// ��������ӵ��������Ϊ��ǰ��ɫ
	EquippedWeapon->SetOwner(Character);
	// ���ý�ɫ�����ƶ������Զ���ת
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	// ���ý�ɫ���ݿ������� Yaw ��ת
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// ����������״̬Ϊ "��װ��"��EWS_Equipped��
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		// ��ȡ��ɫ���ֵĹ������ "RightHandSocket"
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// ���������ӵ���ɫ�����ֲ��
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// ������Ļ�ĳߴ�
	FVector2D ViewportSize;
	// ����������Ϸ�ӿ��Ƿ���Ч��Ȼ���ȡ�ӿڳߴ�
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	// ����ʮ��׼�ǵ�λ�ã���Ļ���ĵ㣩
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2);
	// �������������е�λ�úͷ�������
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	// ����Ļλ�ã�ʮ��׼��λ�ã�ת��Ϊ����λ�úͷ���
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	// �����Ļ����ɹ�ת��Ϊ��������
	if (bScreenToWorld)
	{
		// ����׷�ٵ���ʼλ�ã�ʮ��׼���������е�λ�ã�
		FVector Start = CrosshairWorldPosition;
		// ����׷�ٵ��յ�λ�ã������Ϸ�����������׷�ٳ��ȣ�

		//ǰ��һ�㣬�����������ɫ��ײ
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		// ʹ�ÿɼ���ͨ����������׷�٣�����㵽�յ㣩
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,		// ��������еĽ��
			Start,				// ����׷�ٵ����
			End,				// ����׷�ٵ��յ�
			ECollisionChannel::ECC_Visibility	// �ɼ���ͨ��
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)return;

	// ȷ�� Controller ��Ϊ�գ����Ϊ����� Character ��ȡ��ת��Ϊ ABlasterPlayerController
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		// ȷ�� HUD ��Ϊ�գ����Ϊ����� Controller ��ȡ��ת��Ϊ ABlasterHUD
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			// ����׼����ɢֵ
			// Calculate crosshair spread

			// �ٶȷ�Χ��[0, 600] -> ��Ӧ��ɢϵ����Χ��[0, 1]
			// [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			// ��ȡ��ɫ���ٶ�����������Z���ٶ�����Ϊ0����ȷ��ֻ����ˮƽ�ٶ�
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			// ���ݽ�ɫ��ˮƽ�ٶȼ���׼����ɢ���� CrosshairVelocityFactor
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// �����ɫ�ڿ��У���������׼����ɢ���� CrosshairInAirFactor
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			// �����ɫ�ڵ����ϣ�����ټ�С׼����ɢ���� CrosshairInAirFactor
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// ����������׼�����С׼����ɢ���� CrosshairAimFactor
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			// ������û����׼���� CrosshairAimFactor �𽥹���
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// ���ʱ��׼����ɢ���� CrosshairShootingFactor���𽥹���
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			// �������յ�׼����ɢֵ������ֵ + ������ɢ����
			HUDPackage.CrosshairSpread =
				0.5f +					// ����ֵ
				CrosshairVelocityFactor +  // �ٶ���ɢ����
				CrosshairInAirFactor -	 // ������ɢ����	
				CrosshairAimFactor +		// ��׼��ɢ����
				CrosshairShootingFactor;	// �����ɢ����

			// ������õ� HUDPackage ���͸� HUD ���и���
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}