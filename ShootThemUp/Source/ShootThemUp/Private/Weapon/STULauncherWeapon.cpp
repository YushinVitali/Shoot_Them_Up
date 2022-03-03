// Shoot Them Up Game. All Rights Reserved.

#include "Weapon/STULauncherWeapon.h"
#include "Weapon/STUProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

void ASTULauncherWeapon::OnStartFire()
{
	MakeShot();
}

void ASTULauncherWeapon::MakeShot()
{
	if (!GetWorld())
	{
		return;
	}

	if (IsAmmoEmpty())
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
		return;
	}

	FVector TraceStart;
	FVector TraceEnd;
	if (!GetTraceData(TraceStart, TraceEnd))
	{
		return;
	}

	FHitResult HitResult;
	const FVector EndPoint{MakeHit(HitResult, TraceStart, TraceEnd) ? HitResult.ImpactPoint : TraceEnd};

	const FVector Direction{UKismetMathLibrary::GetDirectionUnitVector(GetMuzzleWorldLocation(), EndPoint)};

	const FTransform SpawnTransform{FRotator::ZeroRotator, GetMuzzleWorldLocation()};
	ASTUProjectile* Projectile{GetWorld()->SpawnActorDeferred<ASTUProjectile>(ProjectileClass, SpawnTransform)};
	if (Projectile)
	{
		Projectile->SetShotDirection(Direction);
		Projectile->SetOwner(GetOwner());
		Projectile->FinishSpawning(SpawnTransform);
	}

	DecreaseAmmo();

	SpawnMuzzleFX();

	UGameplayStatics::SpawnSoundAttached(FireSound, WeaponMesh, MuzzleSocketName);
}