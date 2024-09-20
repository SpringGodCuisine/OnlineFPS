// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// ����������������ۣ�SpringArm�������丽�ӵ���ɫ���������
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;

	// ���������ø�������������丽�ӵ�������۵Ĳ����
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ���ý�ɫ��Yaw�����ϵĿ�����ת
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// ����������ͷ��UI��������ӵ���ɫ�ĸ����
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// ����������ս���������ʹ��ɽ�������ͬ��
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// �����ɫ�׷�
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// ���ý���������������ͨ������ײ��ӦΪ����
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// ���ý�ɫ��������������ͨ���Ϳɼ���ͨ������ײ��ӦΪ����
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	// ���ý�ɫ����ת����
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	// ��ʼ����ɫ��ת��״̬Ϊ��ת��
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// �����������Ƶ��
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// �����Ƹ��ö����������
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			//OnRep_ReplicatedMovement();

			SimProxiesTurn();
			TimeSinceLastMovementReplication = 0.f;

		}
		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Green,
		FString(TEXT("Pressed E"))
	);

	if (Combat)
	{
		//Only Server Call
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEuqipButtonPressed();
		}
	}
}

void ABlasterCharacter::ServerEuqipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health()
{

}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)return FVector();
	return Combat->HitTarget;
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);

}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	// ��ȡ��ɫ���ٶ����������� Z ���ٶ���Ϊ 0���Ժ��Դ�ֱ�����ϵ��ٶ�
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	// �����ɫ��ˮƽ�ٶȴ�С
	float Speed = Velocity.Size();
	return Speed;
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	float Speed = CalculateSpeed();

	// �жϽ�ɫ�Ƿ��ڿ���״̬
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// ����ɫ���ƶ����ڿ���ʱ��������ʼ����׼����
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	}

	// ����ɫ�� Combat �������װ�������Ƿ�Ϊ�գ����Ϊ����ֱ�ӷ���
	if (Combat && Combat->EquippedWeapon == nullptr)return;
	
	// �����ɫ���ھ�ֹ״̬�Ҳ��ڿ��У�վ��������Ծ��
	if (Speed == 0.f && !bIsInAir) // Standing still, not jumping
	{
		bRotateRootBone = true;

		// ��ȡ��ǰ����׼���򣬲���������ʼ��׼����Ĳ�ֵ
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		// �� Yaw ���ϵ���ת��ֵ��ֵ�� AO_Yaw
		AO_Yaw = DeltaAimRotation.Yaw;
		// ���û��ת��δ����ת��״̬��
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			// ��ֵ������׼�� Yaw ֵ
			InterpAO_Yaw = AO_Yaw;
		}

		// ������������� Yaw ��ת
		bUseControllerRotationYaw = true;

		// ����ת���߼�
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running or jumping
	{
		bRotateRootBone = false;
		// ������ʼ����׼����
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// �� AO_Yaw ����Ϊ 0����Ϊ�ڱ��ܻ���Ծ�����迼�� Yaw ƫ��
		AO_Yaw = 0.f;
		// ������������� Yaw ��ת
		bUseControllerRotationYaw = true;
		// ����Ϊδ����ת��״̬
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// ���� Pitch ���ϵ���׼�Ƕȣ�̧ͷ���ͷ��
	AO_Pitch = GetBaseAimRotation().Pitch;
	// ��� Pitch �Ƕȳ��� 90 ���ҽ�ɫ���Ǳ��ؿ��Ƶģ�Զ�̽�ɫ����ӳ���� Pitch ��Χ
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// �� [270, 360] ��Χ�� Pitch ӳ�䵽 [-90, 0]����ȷ���Ƕ���ȷ
		// map pitch from [270, 360] to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)return;

	bRotateRootBone = false;

	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw:: %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// �����ɫ�� Yaw ƫ�������� 90 �ȣ�����Ϊ����ת��
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	// �����ɫ�� Yaw ƫ����С�� -90 �ȣ�����Ϊ����ת��
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	// �����ɫ��ǰ����ת��������δת��״̬��
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		// ʹ�ò�ֵ�����𽥽� InterpAO_Yaw ֵƽ�����ɵ� 0��ȷ��ת��������
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.0f);
		// ��ƽ����� InterpAO_Yaw ֵ��ֵ�� AO_Yaw
		AO_Yaw = InterpAO_Yaw;
		// ��� AO_Yaw �ľ���ֵС�� 15 �ȣ�˵��ת��ӽ����
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			// ���ý�ɫ��ת��״̬Ϊ����ת��
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			// ������ʼ��׼����Ϊ��ǰ�� Yaw ����ȷ������������׼���������µķ���
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
