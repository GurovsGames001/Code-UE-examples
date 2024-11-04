// Best Platform Ever, All Rights Reserved.


#include "Traps/BPEPressurePlate.h"
#include "Components/BoxComponent.h"
#include "Traps/BPEArrowTrap.h"
#include "Character/BPENinjaCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogBPEPressurePlate, All, All);

ABPEPressurePlate::ABPEPressurePlate()
{
	TriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>("TriggerBoxComponent");
	TriggerBoxComponent->SetupAttachment(GetRootComponent());
	TriggerBoxComponent->SetBoxExtent(FVector(200.0f, 200.0f, 15.0f));
	TriggerBoxComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 5.0f));
	TriggerBoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ABPEPressurePlate::BeginPlay()
{
	Super::BeginPlay();

	check(TriggerBoxComponent);

	TriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABPEPressurePlate::OnTriggerBoxComponentBeginOverlap);
}

void ABPEPressurePlate::OnTriggerBoxComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->IsA(ABPENinjaCharacter::StaticClass())) return;

	if (ArrayTraps.Num() == 0)
	{
		UE_LOG(LogBPEPressurePlate, Warning, TEXT("There are no traps attached to the pressure plate %s"), *this->GetName());
	}
	else
	{
		if (FMath::IsNearlyZero(DurationDelayLaunchTraps))
		{
			LaunchTraps();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(LaunchTrapsTimerHandle, this, &ABPEPressurePlate::LaunchTraps, DurationDelayLaunchTraps, false);
		}
	}
}

void ABPEPressurePlate::LaunchTraps() const
{
	for (ABPEArrowTrap* ArrowTrap : ArrayTraps)
	{
		if (ArrowTrap)
		{
			ArrowTrap->LaunchTrap();
		}
	}
}
