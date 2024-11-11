// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BPEWeaponComponent.generated.h"

class ABPEBaseWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BESTPLATFORMEVER_API UBPEWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBPEWeaponComponent();

	void Attack();
	bool IsAttacking() const;

	ABPEBaseWeapon* GetCurrentWeapon() { return CurrentWeapon; };

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") TSubclassOf<ABPEBaseWeapon> WeaponClass;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon") FName WeaponAttachPointName = "WeaponSocket";
	
	virtual void BeginPlay() override;
	
private:
	UPROPERTY() ABPEBaseWeapon* CurrentWeapon = nullptr;
	
	void SpawnWeapon();
};
