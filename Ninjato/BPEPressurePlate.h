// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BPEPressurePlate.generated.h"

class UBoxComponent;
class ABPEArrowTrap;

UCLASS()
class BESTPLATFORMEVER_API ABPEPressurePlate : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	ABPEPressurePlate();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* TriggerBoxComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Delay", meta = (ClampMin = "0.0"))
	float DurationDelayLaunchTraps = 0.0f;

	UPROPERTY(EditInstanceOnly, Category = "Traps")
	TArray<ABPEArrowTrap*> ArrayTraps;

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTriggerBoxComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void LaunchTraps() const;

	FTimerHandle LaunchTrapsTimerHandle;
};
