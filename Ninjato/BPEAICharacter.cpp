// Best Platform Ever, All Rights Reserved.

#include "AI/BPEAICharacter.h"
#include "AI/BPEAIController.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Animations/BPEDamageAnimNotify.h"
#include "Components/BPEWeaponComponent.h"
#include "Weapon/BPEBaseWeapon.h"
#include "Weapon/BPEBow.h"
#include "Weapon/BPEArrow.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BPENinjaCharacter.h"
#include "Animations/BPESoundAttackAnimNotify.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Animations/BPEStopDamageAnimNotify.h"

DEFINE_LOG_CATEGORY_STATIC(LogBPEAICharacter, All, All);

ABPEAICharacter::ABPEAICharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ABPEAIController::StaticClass();

	bUseControllerRotationYaw = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 500.0f);
		GetCharacterMovement()->MaxDepenetrationWithPawn = 0.0f;
	}

	Point1 = CreateDefaultSubobject<USphereComponent>("Point1");
	Point1->SetupAttachment(GetRootComponent());
	Point1->SetRelativeLocation(FVector(250.0f, 0.0f, 0.0f));
	Point1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Point2 = CreateDefaultSubobject<USphereComponent>("Point2");
	Point2->SetupAttachment(GetRootComponent());
	Point2->SetRelativeLocation(FVector(-250.0f, 0.0f, 0.0f));
	Point2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FVector ABPEAICharacter::GetPoint1() const
{
	return Point1->GetComponentLocation();
}

FVector ABPEAICharacter::GetPoint2() const
{
	return Point2->GetComponentLocation();
}

void ABPEAICharacter::StopAttack()
{
	if (WeaponComponent && WeaponComponent->GetCurrentWeapon() && WeaponComponent->IsAttacking())
	{
		StopAnimMontage(MeleeAttackMontage);

		if (CurrentPlayingSoundAttack)
		{
			if (CurrentPlayingSoundAttack->IsPlaying())
			{
				CurrentPlayingSoundAttack->Stop();
			}

			CurrentPlayingSoundAttack->DestroyComponent();
		}

		// if AICharacter is archer
		auto Bow = Cast<ABPEBow>(WeaponComponent->GetCurrentWeapon());
		if (Bow && Bow->GetCurrentArrow())
		{
			Bow->GetCurrentArrow()->Destroy();
		}

		WeaponComponent->GetCurrentWeapon()->StopAttack();
	}
}

void ABPEAICharacter::BeginPlay()
{
	Super::BeginPlay();

	auto DamageNotify = GetDamageNotify<UBPEDamageAnimNotify>(this->MeleeAttackMontage);

	if (DamageNotify)
	{
		DamageNotify->OnNotified.AddUObject(this, &ABPEAICharacter::StartDamage);
	}
	else
	{
		UE_LOG(LogBPEAICharacter, Error, TEXT("Damage notify is forgotten to set"));
		//checkNoEntry();
	}

	auto SoundAttackNotify = GetDamageNotify<UBPESoundAttackAnimNotify>(this->MeleeAttackMontage);

	if (SoundAttackNotify)
	{
		SoundAttackNotify->OnNotified.AddUObject(this, &ABPEAICharacter::PlaySoundAttack);
	}
	else
	{
		UE_LOG(LogBPEAICharacter, Error, TEXT("SoundAttackNotify is forgotten to set"));
		//checkNoEntry();
	}

	auto StopDamageNotify = GetDamageNotify<UBPEStopDamageAnimNotify>(this->MeleeAttackMontage);

	if (StopDamageNotify)
	{
		StopDamageNotify->OnNotified.AddUObject(this, &ABPEAICharacter::StopDamage);
	}
	else
	{
		UE_LOG(LogBPEAICharacter, Error, TEXT("StopDamageNotify is forgotten to set"));
		//checkNoEntry();
	}

	if (!GetMesh()) return;
	this->GetMesh()->SetVisibility(bNoticedByPlayer);

	GetWorld()->GetTimerManager().SetTimer(VisibleTimerHandle, this, &ABPEAICharacter::IncrementCountVisible, DelayDisappearance, true);

	DefaultRotationStandStill = GetActorRotation();

	SafeDistance = FMath::RandRange(MinPercentSafeDistance * SafeDistance, MaxPercentSafeDistance * SafeDistance);
	AttackDistance = FMath::RandRange(MinPercentAttackDistance * AttackDistance, MaxPercentAttackDistance * AttackDistance);
}

void ABPEAICharacter::SetMaxWalkSpeed(const float NewMaxWalkSpeed)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewMaxWalkSpeed;
	}
}

void ABPEAICharacter::StartDamage(USkeletalMeshComponent* MeshComponent)
{
    if (!GetWorld()) return;

	if (MeshComponent != this->GetMesh()) return;

	if (!WeaponComponent->GetCurrentWeapon()) return;

	WeaponComponent->GetCurrentWeapon()->StartDamage();
}

void ABPEAICharacter::StopDamage(USkeletalMeshComponent* MeshComponent)
{
	if (!GetWorld()) return;

	if (MeshComponent != this->GetMesh()) return;

	if (!WeaponComponent->GetCurrentWeapon()) return;

	WeaponComponent->GetCurrentWeapon()->StopDamage();
}

void ABPEAICharacter::PlaySoundAttack(USkeletalMeshComponent* MeshComponent)
{
	if (!GetWorld()) return;

	if (MeshComponent != this->GetMesh()) return;

	CurrentPlayingSoundAttack = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SoundAttack, this->GetActorLocation());
}

void ABPEAICharacter::SetVisibleTrue()
{
	CountVisible = 0.0f;

	if (!bNoticedByPlayer && IsAlive())
	{
		bNoticedByPlayer = true;

		SetVisibilityMeshAndWeapon(true);

		auto PlayerCharacter = Cast<ABPENinjaCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (PlayerCharacter)
		{
			PlayerCharacter->AddNoticedAICharacter(this);
		}
	}
}

void ABPEAICharacter::OnDeath()
{
	Super::OnDeath();
	GetMesh()->AddImpulse(DeathImpulse);
	GetWorld()->GetTimerManager().ClearTimer(VisibleTimerHandle);
	SetVisibilityMeshAndWeapon(true);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	StopAttack();

	// Adding impulse to weapon
	auto Weapon = WeaponComponent->GetCurrentWeapon();
	if (Weapon && Weapon->GetWeaponMesh())
	{
		auto WeaponMesh = Weapon->GetWeaponMesh();

		WeaponMesh->SetWorldLocation(this->GetActorLocation());
		WeaponMesh->AddImpulse(DeathWeaponImpulse);
	}

	bNoticedByPlayer = false;
	auto PlayerCharacter = Cast<ABPENinjaCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerCharacter)
	{
		PlayerCharacter->RemoveNoticedAICharacter(this);
		SetIsAlarm(false);
	}

	RunAdditionalDeathLogic();
}

void ABPEAICharacter::IncrementCountVisible()
{
	CountVisible++;

	if (bNoticedByPlayer && CountVisible > 1)
	{
		bNoticedByPlayer = false;

		SetVisibilityMeshAndWeapon(false);

		auto PlayerCharacter = Cast<ABPENinjaCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (PlayerCharacter)
		{
			PlayerCharacter->RemoveNoticedAICharacter(this);
			SetIsAlarm(false);
		}
	}
}

void ABPEAICharacter::SetVisibilityMeshAndWeapon(const bool VisibilityValue)
{
	if (!GetMesh()) return;
	this->GetMesh()->SetVisibility(VisibilityValue);

	if (!WeaponComponent->GetCurrentWeapon()) return;
	auto Weapon = WeaponComponent->GetCurrentWeapon();

	if (!Weapon->GetWeaponMesh()) return;
	Weapon->GetWeaponMesh()->SetVisibility(VisibilityValue);

	auto Bow = Cast<ABPEBow>(Weapon);
	if (Bow)
	{
		OnVisibilityArrow.ExecuteIfBound(VisibilityValue);
	}
}
