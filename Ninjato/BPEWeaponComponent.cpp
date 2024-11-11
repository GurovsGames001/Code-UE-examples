// Best Platform Ever, All Rights Reserved.


#include "Components/BPEWeaponComponent.h"
#include "Weapon/BPEBaseWeapon.h"
#include "GameFramework/Character.h"
#include "AI/BPEAICharacter.h"

UBPEWeaponComponent::UBPEWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBPEWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnWeapon();
}

void UBPEWeaponComponent::SpawnWeapon()
{
	if (!GetWorld()) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	
	CurrentWeapon = GetWorld()->SpawnActor<ABPEBaseWeapon>(WeaponClass);
	if (!CurrentWeapon) return;

	CurrentWeapon->SetOwner(Character);
	
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	CurrentWeapon->AttachToComponent(Character->GetMesh(), AttachmentRules, WeaponAttachPointName);
}


void UBPEWeaponComponent::Attack()
{
	if (!CurrentWeapon) return;
	CurrentWeapon->Attack();
}

bool UBPEWeaponComponent::IsAttacking() const
{
	return CurrentWeapon && CurrentWeapon->IsAttacking();
}
