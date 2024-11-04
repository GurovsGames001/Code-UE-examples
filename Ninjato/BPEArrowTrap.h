// Best Platform Ever, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BPEArrowTrap.generated.h"

class UArrowComponent;
class ABPEArrow;
class USoundCue;

UCLASS()
class BESTPLATFORMEVER_API ABPEArrowTrap : public AActor
{
	GENERATED_BODY()
	
public:	
	ABPEArrowTrap();

	void LaunchTrap();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UArrowComponent* ArrowComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arrow")
	TSubclassOf<ABPEArrow> ArrowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* SoundLaunchTrap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Arrow")
	float TraceMaxDistance = 1500.0f;

	virtual void BeginPlay() override;

	void GetTraceData(FVector& TraceStart, FVector& TraceEnd) const;
	void MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd, const ABPEArrow* CurrentArrow) const;
};
