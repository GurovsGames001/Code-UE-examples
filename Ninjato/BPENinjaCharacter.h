// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BPEPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "BPENinjaCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_MULTICAST_DELEGATE(FOnFallingSignature);

class USpotLightComponent;
class UAnimMontage;
class UBPEWeaponComponent;
class UBPEDamageAnimNotify;
class USoundCue;
class ABPEAICharacter;

UCLASS(Config=Game)
class BESTPLATFORMEVER_API ABPENinjaCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Input) UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category = Input) UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = Input) UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, Category = Input) UInputAction* MoveBackwardAction;
	UPROPERTY(EditAnywhere, Category = Input) UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, Category = Input) UInputAction* CrouchAction;

public:
	FOnFallingSignature OnFalling;

	explicit ABPENinjaCharacter(const FObjectInitializer& ObjectInitializer);
	FCollisionQueryParams GetQueryParams() const;

	UPROPERTY(EditDefaultsOnly, Category = "Animations") 
	UAnimMontage* MeleeAttackMontage;

	UFUNCTION(BlueprintCallable)
	bool Die();

	UFUNCTION(BlueprintCallable)
	bool IsJumpPressed() const { return bIsJumpPressed; };

	UFUNCTION(BlueprintCallable)
	bool IsCrouchPressed() const { return bIsCrouchPressed; };

	UFUNCTION(BlueprintCallable)
	bool IsReadyWallSlide() const { return bIsReadyWallSlide; };

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	void StartWallSlide();
	bool IsHitBoxArmsOverlapped() const { return bIsHitBoxArmsOverlapped; };

	void AddNoticedAICharacter(ABPEAICharacter* AICharacter) { NoticedAICharacters.Add(AICharacter); }
	void RemoveNoticedAICharacter(ABPEAICharacter* AICharacter) { NoticedAICharacters.Remove(AICharacter); }
	void SetAlarmNoticedAICharacters();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera) 
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera) 
	UCameraComponent* FollowCamera;

	virtual void Jump() override;
	virtual void StopJumping() override;
	virtual void Landed(const FHitResult& Hit) override;

	void MoveForward(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);
	void StopMove() { bIsInputMove = false; CurrentDirectionMove = 0; };
	void CrouchPressed();
	void CrouchUnpressed();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void OnDeath();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void Falling() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components") 
	UBPEHealthComponent* HealthComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Damage") 
	float LifeSpanOnDeath = 5.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") 
	UBPEWeaponComponent* WeaponComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* SoundDeath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* SoundJumpWallSlide;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* SoundStartWallSlide;

	UPROPERTY(BlueprintReadWrite, Category = HitBox)
	bool bIsHitBoxArmsOverlapped = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack")
	bool bEnableRunAttack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SlowingDownSpeedDuringAttack = 0.9f;

private:
	bool bIsCrouchPressed = false;
	bool bIsJumpPressed = false;

	void LineTracingHitBoxArms();
	void LineTracingReadyWallSlide();

	UBPEDamageAnimNotify* GetDamageNotify(UAnimSequenceBase* Animation);
	void StartDamage(USkeletalMeshComponent* MeshComponent);

	TArray<ABPEAICharacter*> NoticedAICharacters;

	// Wall Slide
	bool bIsReadyWallSlide = false;

	bool bWallSlideInProgress = false;
	//float CurrentWallSlidePhase = 0.0f;
	//float PreCurrentWallSlidePhase = 0.0f;

	int32 CurrentNumberWall = 0;
	float LastValueVelocity = 0.0f;

	FTimerHandle FallWallSlideTimerHandle;
	FTimerHandle CurrentNumberWallZeroTimerHandle;

	void StartFallWallSlide();
	void SetCurrentNumberWallZero() { CurrentNumberWall = 0; };

	bool bIsInputMove = false;
	//

	int32 CurrentDirectionMove = 0;

	// Jump in one place
	bool bCanJumpOnePlace = true;
	FTimerHandle CanJumpOnePlaceTimerHandle;
	float DurationPeriodInabilityJumpOnePlace = 0.6f;

	void SetCanJumpOnePlaceTrue() { bCanJumpOnePlace = true; };
	//
};
