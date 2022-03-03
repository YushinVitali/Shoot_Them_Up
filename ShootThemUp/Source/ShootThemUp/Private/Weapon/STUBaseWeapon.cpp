// Shoot Them Up Game. All Rights Reserved.

#include "Weapon/STUBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ASTUBaseWeapon::ASTUBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);
}

void ASTUBaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	check(WeaponMesh);
	checkf(DefaultAmmo.Bullets > 0, TEXT("Bullets count couldn't be less or equal zero!"));
	checkf(DefaultAmmo.Clips > 0, TEXT("Clips count couldn't be less or equal zero!"));

	CurrentAmmo = DefaultAmmo;
}

void ASTUBaseWeapon::OnStartFire() {}

void ASTUBaseWeapon::OnStopFire() {}

void ASTUBaseWeapon::MakeShot() {}

bool ASTUBaseWeapon::GetPlayerViewPoint(FVector& ViewLocation, FRotator& ViewRotation) const
{
	const auto STUCharacter{Cast<ACharacter>(GetOwner())};
	if (!STUCharacter)
	{
		return false;
	}

	if (STUCharacter->IsPlayerControlled())
	{
		const auto Controller{STUCharacter->GetController<APlayerController>()};
		if (!Controller)
		{
			return false;
		}

		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
		return true;
	}
	else
	{
		ViewLocation = GetMuzzleWorldLocation();
		ViewRotation = WeaponMesh->GetSocketRotation(MuzzleSocketName);
		return true;
	}
}

FVector ASTUBaseWeapon::GetMuzzleWorldLocation() const
{
	return WeaponMesh->GetSocketLocation(MuzzleSocketName);
}

bool ASTUBaseWeapon::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{
	FVector ViewLocation;
	FRotator ViewRotation;
	if (!GetPlayerViewPoint(ViewLocation, ViewRotation))
	{
		return false;
	}

	TraceStart = ViewLocation;
	const FVector ShootDirection{ViewRotation.Vector()};
	TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;
	return true;
}

bool ASTUBaseWeapon::IsHittingInFieldOfView(
	const FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd) const
{
	const auto FiringDirection{(TraceEnd - GetMuzzleWorldLocation()).GetSafeNormal()};
	const auto HitDirection{(HitResult.ImpactPoint - GetMuzzleWorldLocation()).GetSafeNormal()};
	const auto DotResult{FVector::DotProduct(FiringDirection, HitDirection)};
	float Angle{FMath::RadiansToDegrees(FMath::Acos(DotResult))};

	UE_LOG(LogBaseWeapon, Display, TEXT("Angle: %f"), Angle);

	return 270 + AngleOfShot <= Angle || Angle <= AngleOfShot;
}

bool ASTUBaseWeapon::MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd)
{
	if (!GetWorld())
	{
		return false;
	}

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	CollisionParams.bReturnPhysicalMaterial = true;

	while (true)
	{
		GetWorld()->LineTraceSingleByChannel(
			HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

		if (!HitResult.bBlockingHit)
		{
			return false;
		}
		if (IsHittingInFieldOfView(HitResult, TraceStart, TraceEnd))
		{
			return true;
		}

		CollisionParams.AddIgnoredActor(HitResult.GetActor());
		continue;
	}

	DecreaseAmmo();
}

void ASTUBaseWeapon::ChangeClip()
{
	if (!CurrentAmmo.IsInfinite)
	{
		if (CurrentAmmo.Clips == 0)
		{
			UE_LOG(LogBaseWeapon, Warning, TEXT("No more clips!"));
			return;
		}
		--CurrentAmmo.Clips;
	}

	CurrentAmmo.Bullets = DefaultAmmo.Bullets;
}

void ASTUBaseWeapon::DecreaseAmmo()
{
	if (CurrentAmmo.Bullets == 0)
	{
		UE_LOG(LogBaseWeapon, Warning, TEXT("Clip is empty!"));
		return;
	}

	--CurrentAmmo.Bullets;

	if (IsClipEmpty() && !IsAmmoEmpty())
	{
		OnStopFire();
		OnClipEmpty.Broadcast(this);
	}
}

bool ASTUBaseWeapon::IsAmmoEmpty() const
{
	return !CurrentAmmo.IsInfinite && CurrentAmmo.Clips == 0 && IsClipEmpty();
}

bool ASTUBaseWeapon::IsClipEmpty() const
{
	return CurrentAmmo.Bullets == 0;
}

bool ASTUBaseWeapon::IsCanReload() const
{
	return CurrentAmmo.Bullets < DefaultAmmo.Bullets && CurrentAmmo.Clips > 0;
}

bool ASTUBaseWeapon::IsAmmoFull() const
{
	return CurrentAmmo.Clips == DefaultAmmo.Clips && //
		   CurrentAmmo.Bullets == DefaultAmmo.Bullets;
}

bool ASTUBaseWeapon::TryToAddAmmo(int32 ClipsAmmount)
{
	if (CurrentAmmo.IsInfinite || IsAmmoFull() || ClipsAmmount <= 0)
	{
		return false;
	}

	if (IsAmmoEmpty())
	{
		UE_LOG(LogBaseWeapon, Display, TEXT("Ammo is empty!"));

		CurrentAmmo.Clips = FMath::Clamp(CurrentAmmo.Clips + ClipsAmmount, 0, DefaultAmmo.Clips + 1);
		OnClipEmpty.Broadcast(this);
	}
	else if (CurrentAmmo.Clips < DefaultAmmo.Clips)
	{
		const auto NextClipsAmount{CurrentAmmo.Clips + ClipsAmmount};
		if (DefaultAmmo.Clips - NextClipsAmount >= 0)
		{
			CurrentAmmo.Clips = NextClipsAmount;

			UE_LOG(LogBaseWeapon, Display, TEXT("Clips were added!"));
		}
		else
		{
			CurrentAmmo.Clips = DefaultAmmo.Clips;

			UE_LOG(LogBaseWeapon, Display, TEXT("Ammo is full now!"));
		}
	}
	else
	{
		UE_LOG(LogBaseWeapon, Display, TEXT("Bullets not added!"));
		return false;
	}

	return true;
}

UNiagaraComponent* ASTUBaseWeapon::SpawnMuzzleFX()
{
	return UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFX, //
		WeaponMesh,												  //
		MuzzleSocketName,										  //
		FVector::ZeroVector,									  //
		FRotator::ZeroRotator,									  //
		EAttachLocation::SnapToTarget, true);
}