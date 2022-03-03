// Shoot Them Up Game. All Rights Reserved.

#include "Components/STUHealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraShake.h"
#include "STUGameModeBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Perception/AISense_Damage.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthComponent, All, All)

USTUHealthComponent::USTUHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USTUHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0, TEXT("The maximum number of health cannot be less than zero!"));

	SetHealth(MaxHealth);

	AActor* ComponentOwner{GetOwner()};
	if (ComponentOwner)
	{
		ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &USTUHealthComponent::OnTakeAnyDamage);
		ComponentOwner->OnTakePointDamage.AddDynamic(this, &USTUHealthComponent::OnTakePointDamage);
		ComponentOwner->OnTakeRadialDamage.AddDynamic(this, &USTUHealthComponent::OnTakeRadialDamage);
	}
}

void USTUHealthComponent::OnTakeAnyDamage(
	AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogHealthComponent, Display, TEXT("On any damage: %f"), Damage);
}

void USTUHealthComponent::OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy,
	FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const class UDamageType* DamageType, AActor* DamageCauser)
{
	const auto FinalDamage{Damage * GetPointDamageModifier(GetOwner(), BoneName)};

	UE_LOG(LogHealthComponent, Display, TEXT("On point damage: %f, %f, bone: %s"), Damage, FinalDamage,
		*BoneName.ToString());

	ApplyDamage(FinalDamage, InstigatedBy);
}

void USTUHealthComponent::OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogHealthComponent, Display, TEXT("On radial damage: %f"), Damage);

	ApplyDamage(Damage, InstigatedBy);
}

void USTUHealthComponent::HealUpdate()
{
	SetHealth(Health + HealthModifier);

	if (IsHealthFull() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HealTimerHandler);
	}
}

void USTUHealthComponent::SetHealth(float NewHealth)
{
	const auto NextHealth{FMath::Clamp(NewHealth, 0.0f, MaxHealth)};
	const auto HealthDelta{NextHealth - Health};

	Health = NextHealth;
	OnHealthChanged.Broadcast(Health, HealthDelta);

	UE_LOG(LogHealthComponent, Display, TEXT("SetHealth: %f"), Health);
}

bool USTUHealthComponent::TryToAddHealthPercent(float HealthPercent)
{
	if (IsDead() || IsHealthFull())
	{
		return false;
	}

	const auto AddedAmountOfHealth{MaxHealth * (HealthPercent / 100.0f)};
	SetHealth(Health + AddedAmountOfHealth);
	return true;
}

bool USTUHealthComponent::IsHealthFull() const
{
	return FMath::IsNearlyEqual(Health, MaxHealth);
}

void USTUHealthComponent::PlayCameraShake()
{
	if (IsDead())
	{
		return;
	}
	const auto Player{Cast<APawn>(GetOwner())};
	if (!Player)
	{
		return;
	}

	const auto Controller{Player->GetController<APlayerController>()};
	if (!Controller)
	{
		return;
	}
	if (!Controller->PlayerCameraManager)
	{
		return;
	}

	Controller->PlayerCameraManager->StartCameraShake(CameraShake);
}

void USTUHealthComponent::Killed(AController* KillerController)
{
	if (!GetWorld())
	{
		return;
	}

	const auto GameMode{Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode())};
	if (!GameMode)
	{
		return;
	}

	const auto Player{Cast<APawn>(GetOwner())};
	const auto VictimController{Player ? Player->Controller : nullptr};

	GameMode->Killed(KillerController, VictimController);
}

void USTUHealthComponent::ApplyDamage(float Damage, AController* InstigatedBy)
{
	if (Damage <= 0.0f || IsDead() || !GetWorld())
	{
		return;
	}

	SetHealth(Health - Damage);

	GetWorld()->GetTimerManager().ClearTimer(HealTimerHandler);

	if (IsDead())
	{
		Killed(InstigatedBy);
		OnDeath.Broadcast();
	}
	else
	{
		if (IsAutoHeal)
		{
			GetWorld()->GetTimerManager().SetTimer(
				HealTimerHandler, this, &USTUHealthComponent::HealUpdate, HealthUpdateTime, true, HealthDelay);
		}
	}

	PlayCameraShake();
	ReportDamageEvent(Damage, InstigatedBy);
}

float USTUHealthComponent::GetPointDamageModifier(AActor* DamageActor, const FName& BoneName)
{
	const auto Character{Cast<ACharacter>(DamageActor)};
	if (!Character ||			 //
		!Character->GetMesh() || //
		!Character->GetMesh()->GetBodyInstance(BoneName))
	{
		return 1.0f;
	}

	const auto PhysMaterial{Character->GetMesh()->GetBodyInstance(BoneName)->GetSimplePhysicalMaterial()};

	return !DamageModifiers.Contains(PhysMaterial) ? 1.0f : DamageModifiers[PhysMaterial];
}

void USTUHealthComponent::ReportDamageEvent(float Damage, AController* InstigatedBy)
{
	if (!InstigatedBy || !InstigatedBy->GetPawn() || !GetOwner())
	{
		return;
	}

	UAISense_Damage::ReportDamageEvent(GetWorld(),	 //
		GetOwner(),									 //
		InstigatedBy->GetPawn(),					 //
		Damage,										 //
		InstigatedBy->GetPawn()->GetActorLocation(), //
		GetOwner()->GetActorLocation());
}