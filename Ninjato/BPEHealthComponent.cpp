// Best Platform Ever, All Rights Reserved.

#include "Components/BPEHealthComponent.h"
#include "Engine/World.h"
#include "Gameframework/Actor.h"
#include "Gameframework/Controller.h"
#include "Gameframework/Pawn.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthComponent, All, All);

UBPEHealthComponent::UBPEHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBPEHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    check(MaxNumberLives > 0);

    SetHealth(MaxNumberLives);

    AActor* ComponentOwner = GetOwner();
    if (ComponentOwner)
    {
        ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &UBPEHealthComponent::OnTakeAnyDamage);
    }
}

void UBPEHealthComponent::OnTakeAnyDamage(
    AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || IsDead() || !GetWorld()) return;

    SetHealth(Health - Damage);

    if (WoundEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WoundEffect, GetOwner()->GetActorLocation(), FRotator(-90.0f, 0.0f, 0.0f), true);
    }

    if (IsDead())
    {
        OnDeath.Broadcast();
    }
}

void UBPEHealthComponent::SetHealth(const float NewHealth)
{
    const auto NextHealth = FMath::Clamp(NewHealth, 0.0f, MaxNumberLives);
    const auto HealthDelta = NextHealth - Health;

    Health = NextHealth;
    OnHealthChanged.Broadcast(Health, HealthDelta);

    UE_LOG(LogHealthComponent, Display, TEXT("Number of lives: %f"), Health);
}

bool UBPEHealthComponent::TryToAddHealth(const int32 HealthAmount)
{
    if (IsHealthFull() || IsDead() || HealthAmount <= 0) return false;

    SetHealth(Health + HealthAmount);
    return true;
}

bool UBPEHealthComponent::IsHealthFull() const
{
    return FMath::IsNearlyEqual(Health, MaxNumberLives);
}

