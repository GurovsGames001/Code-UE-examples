// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BPEHealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathSignature);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, float);

class UParticleSystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BESTPLATFORMEVER_API UBPEHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UBPEHealthComponent();

    FOnDeathSignature OnDeath;
    FOnHealthChangedSignature OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const { return FMath::IsNearlyZero(Health); };

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercent() { return Health / MaxNumberLives; };

    float GetHealth() const { return Health; };

    bool TryToAddHealth(int32 HealthAmount);
    bool IsHealthFull() const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxNumberLives = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    UParticleSystem* WoundEffect;

    virtual void BeginPlay() override;

private:
    float Health = 0.0f;

    UFUNCTION()
    void OnTakeAnyDamage(
        AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void SetHealth(float NewHealth);
};
