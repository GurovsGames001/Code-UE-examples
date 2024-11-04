// Best Platform Ever, All Rights Reserved.

#include "Character/BPENinjaCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/BPENinjaCharacterMovement.h"
#include "Components/BPEHealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Game/BPEGameModeBase.h"
#include "Components/BPEWeaponComponent.h"
#include "Animations/BPEDamageAnimNotify.h"
#include "Weapon/BPEBaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "AI/BPEAICharacter.h"
#include "Sound/SoundCue.h"

DEFINE_LOG_CATEGORY_STATIC(LogBPENinjaCharacter, All, All);

ABPENinjaCharacter::ABPENinjaCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UBPENinjaCharacterMovement>(CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(25.0f, 95.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	JumpMaxHoldTime = 0.3f;

	constexpr float TargetArmLength = 1000.f;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetWorldRotation(FRotator(0.0f, -270.0f, 0.0f));
	CameraBoom->TargetArmLength = TargetArmLength;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 410.0f));
	FollowCamera->SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UBPEHealthComponent>("HealthComponent");

	WeaponComponent = CreateDefaultSubobject<UBPEWeaponComponent>("WeaponComponent");
}

FCollisionQueryParams ABPENinjaCharacter::GetQueryParams() const
{
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(CharacterChildren);
	QueryParams.AddIgnoredActor(this);

	return QueryParams;
}

void ABPENinjaCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (const auto* PlayerController = Cast<APlayerController>(Controller))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	check(HealthComponent);
	check(WeaponComponent);
	HealthComponent->OnDeath.AddUObject(this, &ABPENinjaCharacter::OnDeath);

	auto DamageNotify = GetDamageNotify(this->MeleeAttackMontage);

	if (DamageNotify)
	{
		DamageNotify->OnNotified.AddUObject(this, &ABPENinjaCharacter::StartDamage);
	}
	else
	{
		UE_LOG(LogBPENinjaCharacter, Error, TEXT("Damage notify is forgotten to set"));
		//checkNoEntry();
	}
}

void ABPENinjaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	LineTracingHitBoxArms();

	if (!FMath::IsNearlyZero(GetVelocity().X))
	{
		LastValueVelocity = GetVelocity().X;
	}

	//LineTracingReadyWallSlide();
}

bool ABPENinjaCharacter::IsAlive() const
{
	return !HealthComponent->IsDead();;
}

void ABPENinjaCharacter::StartWallSlide()
{
	bWallSlideInProgress = true;

	// Get current number wall
	if (LastValueVelocity > 0.0f)
	{
		CurrentNumberWall = 1;
	}
	else
	{
		CurrentNumberWall = 2;
	}

	UE_LOG(LogBPENinjaCharacter, Display, TEXT("CurrentNumberWall = %d"), CurrentNumberWall);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundStartWallSlide, this->GetActorLocation());
}

void ABPENinjaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(WeaponComponent)

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABPENinjaCharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABPENinjaCharacter::StopJumping);

			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ABPENinjaCharacter::MoveForward);
			EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &ABPENinjaCharacter::MoveBackward);
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &ABPENinjaCharacter::StopMove);
			EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Completed, this, &ABPENinjaCharacter::StopMove);

			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, WeaponComponent, &UBPEWeaponComponent::Attack);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABPENinjaCharacter::CrouchPressed);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ABPENinjaCharacter::CrouchUnpressed);
		}
}

void ABPENinjaCharacter::Jump()
{
	if (WeaponComponent->IsAttacking()) return;

	// Check whether it is possible to jump in one place
	if (GetMovementComponent() && FMath::IsNearlyZero(GetMovementComponent()->Velocity.X) && !bWallSlideInProgress)
	{
		if (bCanJumpOnePlace) bCanJumpOnePlace = false;
		else return;
	}

	// Check possible input move during wall slide
	if (bWallSlideInProgress)
	{
		GetWorld()->GetTimerManager().SetTimer(FallWallSlideTimerHandle, this, &ABPENinjaCharacter::StartFallWallSlide, 0.1f);
	}

	if (GetMovementComponent() && GetMovementComponent()->IsCrouching())
	{
		this->UnCrouch();
	}

	const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

	if (NinjaCharacterMovement->IsWallSliding())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundJumpWallSlide, this->GetActorLocation());
	}

	bIsJumpPressed = true;
	Super::Jump();
}

void ABPENinjaCharacter::StopJumping()
{
	bIsJumpPressed = false;

	// Start timer to set bCanJumpOnePlace to true
	if (!bCanJumpOnePlace && GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(CanJumpOnePlaceTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(CanJumpOnePlaceTimerHandle, this, 
			&ABPENinjaCharacter::SetCanJumpOnePlaceTrue, DurationPeriodInabilityJumpOnePlace);
	}
	
	Super::StopJumping();
}

void ABPENinjaCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bIsCrouchPressed)
	{
		this->Crouch();
	}

	bWallSlideInProgress = false;
	//CurrentWallSlidePhase = 0.0f;
	//PreCurrentWallSlidePhase = 0.0f;

	CurrentNumberWall = 0;

	UE_LOG(LogBPENinjaCharacter, Display, TEXT("Player is landed"));
}

void ABPENinjaCharacter::MoveForward(const FInputActionValue& Value)
{
	if (!Controller || (!bEnableRunAttack && WeaponComponent->IsAttacking())) return; //|| WeaponComponent->IsAttacking()

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Ignore the input if the character is already moving
	if (CurrentDirectionMove == 0) CurrentDirectionMove = 1;
	else if (CurrentDirectionMove == -1) return;

	if (!bIsInputMove)
	{
		bIsInputMove = true;
	}

	const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

	// It's movement logic for wall sliding
	if (bWallSlideInProgress && CurrentNumberWall == 2 && NinjaCharacterMovement->Velocity.Z > 0.0f)
	{
		bIsInputMove = false;
		return;
	}

	// It's movement logic for wall sliding
	/*if (GetMovementComponent())
	{
		const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

		if (NinjaCharacterMovement->IsFalling() && MovementVector.Y != CurrentWallSlidePhase)
		{
			PreCurrentWallSlidePhase = CurrentWallSlidePhase;
			CurrentWallSlidePhase = MovementVector.Y;
		}
		else if (bWallSlideInProgress && PreCurrentWallSlidePhase == CurrentWallSlidePhase && GetVelocity().Z > 0.0f)
		{
			return;
		}
		else if (NinjaCharacterMovement->IsWallSliding())
		{
			PreCurrentWallSlidePhase = CurrentWallSlidePhase;
		}
	}*/

	if (bEnableRunAttack && WeaponComponent->IsAttacking())
	{
		AddMovementInput(ForwardDirection, MovementVector.Y * SlowingDownSpeedDuringAttack);
	}
	else
	{
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
}

void ABPENinjaCharacter::MoveBackward(const FInputActionValue& Value)
{
	if (!Controller || (!bEnableRunAttack && WeaponComponent->IsAttacking())) return; //|| WeaponComponent->IsAttacking()

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Ignore the input if the character is already moving
	if (CurrentDirectionMove == 0) CurrentDirectionMove = -1;
	else if (CurrentDirectionMove == 1) return;

	if (!bIsInputMove)
	{
		bIsInputMove = true;
	}

	const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

	// It's movement logic for wall sliding
	if (bWallSlideInProgress && CurrentNumberWall == 1 && NinjaCharacterMovement->Velocity.Z > 0.0f)
	{
		bIsInputMove = false;
		return;
	}

	// It's movement logic for wall sliding
	/*if (GetMovementComponent())
	{
		const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

		if (NinjaCharacterMovement->IsFalling() && MovementVector.Y != CurrentWallSlidePhase)
		{
			PreCurrentWallSlidePhase = CurrentWallSlidePhase;
			CurrentWallSlidePhase = MovementVector.Y;
		}
		else if (bWallSlideInProgress && PreCurrentWallSlidePhase == CurrentWallSlidePhase && GetVelocity().Z > 0.0f)
		{
			return;
		}
		else if (NinjaCharacterMovement->IsWallSliding())
		{
			PreCurrentWallSlidePhase = CurrentWallSlidePhase;
		}
	}*/

	if (bEnableRunAttack && WeaponComponent->IsAttacking())
	{
		AddMovementInput(ForwardDirection, MovementVector.Y * SlowingDownSpeedDuringAttack);
	}
	else
	{
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}
}

void ABPENinjaCharacter::CrouchPressed()
{
	if (!GetMovementComponent() || GetMovementComponent()->IsFalling() || bWallSlideInProgress) return;
	bIsCrouchPressed = true;
	this->Crouch();
}

void ABPENinjaCharacter::CrouchUnpressed()
{
	bIsCrouchPressed = false;
	this->UnCrouch();
}

void ABPENinjaCharacter::OnDeath()
{
	UE_LOG(LogBPENinjaCharacter, Display, TEXT("Player %s is dead"), *GetName());

	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(LifeSpanOnDeath);

	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	// Changing the logic of a dead ninja's weapon
	auto Weapon = WeaponComponent->GetCurrentWeapon();
	if (Weapon && Weapon->GetWeaponMesh())
	{
		auto WeaponMesh = Weapon->GetWeaponMesh();

		// Changing the weapon collision
		WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

		// Enabling weapon physics
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetSimulatePhysics(true);
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	if (GetWorld() && SoundDeath)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundDeath);
	}
}

bool ABPENinjaCharacter::Die()
{
	float Damage = HealthComponent->GetHealth();
	this->TakeDamage(Damage, FDamageEvent{}, this->Controller, this);
	return true;
}

void ABPENinjaCharacter::Falling()
{
	OnFalling.Broadcast();
	//bWallSlideInProgress = false;
}

void ABPENinjaCharacter::LineTracingHitBoxArms()
{
	const auto World = GetWorld();
	if (!World) return;

	int32 i = 0;

	for (i; i < 5; i++)
	{
		FVector StartPoint = GetActorLocation() + FVector(0.0f, 0.0f, i * 10);
		FVector EndPoint = StartPoint + GetActorForwardVector() * 100;
		FHitResult HitBoxArmsHitResult;
		DrawDebugLine(World, StartPoint, EndPoint, FColor::Blue, false, .5f, 0, 1.f);
		bIsHitBoxArmsOverlapped = World->LineTraceSingleByChannel(HitBoxArmsHitResult, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility);

		/*StartPoint = GetActorLocation() + FVector(0.0f, 0.0f, i * 10);
		EndPoint = StartPoint + GetActorForwardVector() * 100;
		HitBoxArmsHitResult;
		DrawDebugLine(World, StartPoint, EndPoint, FColor::Blue, false, .5f, 0, 1.f);
		bIsHitBoxArmsOverlapped = bIsHitBoxArmsOverlapped && World->LineTraceSingleByChannel(HitBoxArmsHitResult, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility);*/
	}
}

void ABPENinjaCharacter::LineTracingReadyWallSlide()
{
	const auto World = GetWorld();
	if (!World) return;

	FVector StartPoint = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector EndPoint = StartPoint + GetActorForwardVector() * 100;
	FHitResult HitBoxArmsHitResult;
	DrawDebugLine(World, StartPoint, EndPoint, FColor::Cyan, false, .5f, 0, 1.f);
	bIsReadyWallSlide = World->LineTraceSingleByChannel(HitBoxArmsHitResult, StartPoint, EndPoint, ECollisionChannel::ECC_Visibility);
}

UBPEDamageAnimNotify* ABPENinjaCharacter::GetDamageNotify(UAnimSequenceBase* Animation)
{
	if (!Animation) return nullptr;

	const auto NotifyEvents = Animation->Notifies;
	for (auto NotifyEvent : NotifyEvents)
	{
		auto AnimNotify = Cast<UBPEDamageAnimNotify>(NotifyEvent.Notify);
		if (AnimNotify)
		{
			return AnimNotify;
		}
	}
	return nullptr;
}

void ABPENinjaCharacter::StartDamage(USkeletalMeshComponent* MeshComponent)
{
	if (!GetWorld()) return;

	if (MeshComponent != this->GetMesh()) return;

	WeaponComponent->GetCurrentWeapon()->StartDamage();
}

void ABPENinjaCharacter::SetAlarmNoticedAICharacters()
{
	if (NoticedAICharacters.Num() == 0) return;

	for (ABPEAICharacter* AICharacter : NoticedAICharacters)
	{
		AICharacter->SetIsAlarm(true);
	}
}

void ABPENinjaCharacter::StartFallWallSlide()
{
	if (!bIsInputMove)
	{
		const auto NinjaCharacterMovement = Cast<UBPENinjaCharacterMovement>(GetMovementComponent());

		/*if (NinjaCharacterMovement)
		{
			NinjaCharacterMovement->Velocity.X = 0.0f;
			StopJumping();
		}*/

		GetWorld()->GetTimerManager().SetTimer(CurrentNumberWallZeroTimerHandle, this, &ABPENinjaCharacter::SetCurrentNumberWallZero, 0.3f);
	}
}
