// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/BPEBaseCharacter.h"
#include "BPEAICharacter.generated.h"

class UBehaviorTree;
class USphereComponent;
class UBPEDamageAnimNotify;
class USoundCue;
class UAudioComponent;

DECLARE_DELEGATE_OneParam(FOnVisibilityArrowSignature, bool);

UCLASS()
class BESTPLATFORMEVER_API ABPEAICharacter : public ABPEBaseCharacter
{
	GENERATED_BODY()

public:
	ABPEAICharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	UFUNCTION(BlueprintCallable)
	bool GetAttackModeInProgress() const { return AttackModeInProgress; };
	void SetAttackModeInProgress(const bool NewFlag) { AttackModeInProgress = NewFlag; };

	UFUNCTION(BlueprintCallable)
	bool GetIsFoundEnemy() const { return IsFoundEnemy; };
	void SetIsFoundEnemy(const bool NewFlag) { IsFoundEnemy = NewFlag; };

	UFUNCTION(BlueprintCallable)
	void SetMaxPatrollingSpeed() { SetMaxWalkSpeed(MaxPatrollingSpeed); };

	UFUNCTION(BlueprintCallable)
	void SetMaxDetectionSpeed() { SetMaxWalkSpeed(MaxDetectionSpeed); };

	UFUNCTION(BlueprintCallable)
	void SetMaxAttackSpeed() { SetMaxWalkSpeed(MaxAttackSpeed); };

	UFUNCTION(BlueprintCallable)
	FVector GetPoint1() const;

	UFUNCTION(BlueprintCallable)
	FVector GetPoint2() const;

	bool GetStandStillInPatrolling() const { return bStandStillInPatrolling; };

	float GetSafeDistance() const { return SafeDistance; };
	float GetAttackDistance() const { return AttackDistance; };

	UFUNCTION(BlueprintCallable)
	bool IsbNoticedByPlayer() const { return bNoticedByPlayer; };

	void SetIsAlarm(bool NewValue) { bIsAlarm = NewValue; };
	bool GetIsAlarm() const { return bIsAlarm; };

	void StopAttack();

	FOnVisibilityArrowSignature OnVisibilityArrow;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points")
	USphereComponent* Point1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Points")
	USphereComponent* Point2;

	// Speeds

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0.0"))
	float MaxPatrollingSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0.0"))
	float MaxDetectionSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0.0"))
	float MaxAttackSpeed = 600.0f;

	UPROPERTY(EditInstanceOnly, Category = "Patrolling")
	bool bStandStillInPatrolling = false;

	UPROPERTY(EditDefaultsOnly, Category = "Visibility", meta = (ClampMin = "0.0"))
	float DelayDisappearance = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	FVector DeathImpulse = FVector(0.0f, 30000.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	FVector DeathWeaponImpulse = FVector(0.0f, 7000.0f, 0.0f);

	// Safe Distance

	UPROPERTY(EditDefaultsOnly, Category = "Safe Distance", meta = (ClampMin = "0.0"))
	float SafeDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Safe Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinPercentSafeDistance = 0.85;

	UPROPERTY(EditDefaultsOnly, Category = "Safe Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxPercentSafeDistance = 1.0f;

	// Attack Distance

	UPROPERTY(EditDefaultsOnly, Category = "Attack Distance", meta = (ClampMin = "0.0"))
	float AttackDistance = 100;

	UPROPERTY(EditDefaultsOnly, Category = "Attack Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinPercentAttackDistance = 0.85;

	UPROPERTY(EditDefaultsOnly, Category = "Attack Distance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxPercentAttackDistance = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* SoundAttack;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetVisibleTrue();

	UFUNCTION(BlueprintCallable)
	FRotator GetDefaultRotationStandStill() const { return DefaultRotationStandStill; };

	virtual void OnDeath() override;

	UFUNCTION(BlueprintImplementableEvent)
	void RunAdditionalDeathLogic();

	virtual void StartDamage(USkeletalMeshComponent* MeshComponent);
	virtual void StopDamage(USkeletalMeshComponent* MeshComponent);
	virtual void PlaySoundAttack(USkeletalMeshComponent* MeshComponent);

private:
	bool AttackModeInProgress = false;
	bool IsFoundEnemy = false;

	bool bIsAlarm = false;

	void SetMaxWalkSpeed(const float NewMaxWalkSpeed);

	float CountVisible = 0.0f;
	FTimerHandle VisibleTimerHandle;
	bool bNoticedByPlayer = false;

	UPROPERTY()
	UAudioComponent* CurrentPlayingSoundAttack;

	void IncrementCountVisible();
	void SetVisibilityMeshAndWeapon(const bool VisibilityValue);

	FRotator DefaultRotationStandStill;
	
};

template <typename T>
T* GetDamageNotify(UAnimSequenceBase* Animation)
{
	if (!Animation) return nullptr;

	const auto NotifyEvents = Animation->Notifies;
	for (auto NotifyEvent : NotifyEvents)
	{
		auto AnimNotify = Cast<T>(NotifyEvent.Notify);
		if (AnimNotify)
		{
			return AnimNotify;
		}
	}
	return nullptr;
}
