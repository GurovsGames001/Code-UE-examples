// Best Platform Ever, All Rights Reserved.


#include "Traps/BPEArrowTrap.h"
#include "Components/ArrowComponent.h"
#include "Weapon/BPEArrow.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

DEFINE_LOG_CATEGORY_STATIC(LogBPEArrowTrap, All, All);

ABPEArrowTrap::ABPEArrowTrap()
{
	PrimaryActorTick.bCanEverTick = false;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
	ArrowComponent->SetupAttachment(GetRootComponent());
}

void ABPEArrowTrap::LaunchTrap()
{
	UE_LOG(LogBPEArrowTrap, Display, TEXT("The trap %s launched"), *this->GetName());

	const FTransform SpawnTransform(ArrowComponent->GetRelativeRotation(), GetActorLocation());

	ABPEArrow* Arrow = GetWorld()->SpawnActorDeferred<ABPEArrow>(ArrowClass, SpawnTransform);

	if (Arrow)
	{
		Arrow->SetVisibilityArrowMesh(false);

		FVector TraceStart, TraceEnd;
		GetTraceData(TraceStart, TraceEnd);

		FHitResult HitResult;
		MakeHit(HitResult, TraceStart, TraceEnd, Arrow);

		const FVector EndPoint = HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd;
		const FVector Direction = (EndPoint - GetActorLocation()).GetSafeNormal();

		Arrow->SetShotDirection(Direction);

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundLaunchTrap, this->GetActorLocation());
		Arrow->FinishSpawning(SpawnTransform);
	}
}

void ABPEArrowTrap::BeginPlay()
{
	Super::BeginPlay();
}

void ABPEArrowTrap::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{
	TraceStart = GetActorLocation();
	const FVector ShootDirection = (ArrowComponent->GetRelativeRotation()).Vector();
	TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;
}

void ABPEArrowTrap::MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd, const ABPEArrow* CurrentArrow) const
{
	if (!GetWorld()) return;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	CollisionParams.AddIgnoredActor(CurrentArrow);
	CollisionParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
}

