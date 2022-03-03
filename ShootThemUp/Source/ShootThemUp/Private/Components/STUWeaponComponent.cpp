// Shoot Them Up Game. All Rights Reserved.

#include "Components/STUWeaponComponent.h"
#include "Weapon/STUBaseWeapon.h"
#include "GameFramework/Character.h"
#include "Animations/STUAnimNotify.h"
#include "Animations/STUEquipFinishedAnimNotify.h"
#include "Animations/STUReloadFinishedAnimNotify.h"
#include "Animations/AnimUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogWeaponComponent, All, All);

constexpr int32 WeaponNum{2};

USTUWeaponComponent::USTUWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USTUWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(WeaponData.Num() == WeaponNum, TEXT("Our character can hold only %i weapon items!"), WeaponNum);

	InitAnimations();
	SpawnWeapons();

	EquipWeapon(CurrentWeaponIndex);
}

void USTUWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CurrentWeapon = nullptr;
	for (auto Weapon : Weapons)
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Weapon->Destroy();
	}
	Weapons.Empty();

	Super::EndPlay(EndPlayReason);
}

void USTUWeaponComponent::OnStartFire()
{
	if (!IsCanFire())
	{
		return;
	}

	CurrentWeapon->OnStartFire();
}

void USTUWeaponComponent::OnStopFire()
{
	if (!CurrentWeapon)
	{
		return;
	}

	CurrentWeapon->OnStopFire();
}

void USTUWeaponComponent::OnNextWeapon()
{
	if (!IsCanEquip())
	{
		return;
	}

	CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
	EquipWeapon(CurrentWeaponIndex);
}

void USTUWeaponComponent::OnReload()
{
	ChangeClip();
}

bool USTUWeaponComponent::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
	if (CurrentWeapon)
	{
		UIData = CurrentWeapon->GetUIData();
		return true;
	}

	return false;
}

bool USTUWeaponComponent::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
	if (CurrentWeapon)
	{
		AmmoData = CurrentWeapon->GetAmmoData();
		return true;
	}

	return false;
}

void USTUWeaponComponent::SpawnWeapons()
{
	ACharacter* Character{Cast<ACharacter>(GetOwner())};
	if (!GetWorld() && !Character)
	{
		return;
	}

	for (auto OneWeaponData : WeaponData)
	{
		auto Weapon = GetWorld()->SpawnActor<ASTUBaseWeapon>(OneWeaponData.WeaponClass);
		if (!Weapon)
		{
			continue;
		}
		Weapon->OnClipEmpty.AddUObject(this, &USTUWeaponComponent::OnClipEmpty);
		Weapon->SetOwner(Character);
		Weapons.Add(Weapon);

		AttachWeaponToSocket(Weapon, Character->GetMesh(), WeaponArmorySocketName);
	}
}

void USTUWeaponComponent::AttachWeaponToSocket(
	ASTUBaseWeapon* Weapon, USceneComponent* SceneComponent, const FName& SocketName)
{
	if (!Weapon || !SceneComponent)
	{
		return;
	}
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	Weapon->AttachToComponent(SceneComponent, AttachmentRules, SocketName);
}

void USTUWeaponComponent::EquipWeapon(int32 WeaponIndex)
{
	if (WeaponIndex < 0 || WeaponIndex >= Weapons.Num())
	{
		UE_LOG(LogWeaponComponent, Warning, TEXT("Invalid weapon index!"));
		return;
	}

	ACharacter* Character{Cast<ACharacter>(GetOwner())};
	if (!Character)
	{
		return;
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->Zoom(false);
		CurrentWeapon->OnStopFire();
		AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponArmorySocketName);
	}

	CurrentWeapon = Weapons[WeaponIndex];
	const auto CurrentWeaponData{WeaponData.FindByPredicate([&](const FWeaponData& Data) { //
		return Data.WeaponClass == CurrentWeapon->GetClass();							   //
	})};
	CurrentReloadAnimMontage = CurrentWeaponData ? CurrentWeaponData->ReloadAnimMontage : nullptr;

	AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);

	IsEquipAnimInProgress = true;
	PlayAnimMontage(EquipAnimMontage);
}

void USTUWeaponComponent::PlayAnimMontage(UAnimMontage* Animation)
{
	ACharacter* Character{Cast<ACharacter>(GetOwner())};
	if (!Character)
	{
		return;
	}

	Character->PlayAnimMontage(Animation);
}

void USTUWeaponComponent::InitAnimations()
{
	auto EquipFinishedNotify{AnimUtils::FindNotifyByClass<USTUEquipFinishedAnimNotify>(EquipAnimMontage)};
	if (EquipFinishedNotify)
	{
		EquipFinishedNotify->OnNotifed.AddUObject(this, &USTUWeaponComponent::OnEquipFinished);
	}
	else
	{
		UE_LOG(LogWeaponComponent, Error, TEXT("Equip anim notify is forgotten to set!"));
		checkNoEntry();
	}

	for (auto OneWeaponData : WeaponData)
	{
		auto ReloadFinishedNotify{
			AnimUtils::FindNotifyByClass<USTUReloadFinishedAnimNotify>(OneWeaponData.ReloadAnimMontage)};
		if (!ReloadFinishedNotify)
		{
			UE_LOG(LogWeaponComponent, Error, TEXT("Reload anim notify is forgotten to set!"));
			checkNoEntry();
		}
		ReloadFinishedNotify->OnNotifed.AddUObject(this, &USTUWeaponComponent::OnReloadFinished);
	}
}

void USTUWeaponComponent::OnEquipFinished(USkeletalMeshComponent* MeshComponent)
{
	ACharacter* Character{Cast<ACharacter>(GetOwner())};
	if (!MeshComponent && !Character || Character->GetMesh() != MeshComponent)
	{
		return;
	}

	IsEquipAnimInProgress = false;
}

void USTUWeaponComponent::OnReloadFinished(USkeletalMeshComponent* MeshComponent)
{
	ACharacter* Character{Cast<ACharacter>(GetOwner())};
	if (!MeshComponent && !Character || Character->GetMesh() != MeshComponent)
	{
		return;
	}

	IsReloadAnimInProgress = false;
}

bool USTUWeaponComponent::IsCanFire() const
{
	return CurrentWeapon && !IsEquipAnimInProgress && !IsReloadAnimInProgress;
}

bool USTUWeaponComponent::IsCanEquip() const
{
	return !IsEquipAnimInProgress && !IsReloadAnimInProgress;
}

bool USTUWeaponComponent::IsCanReload() const
{
	return CurrentWeapon			  //
		   && !IsReloadAnimInProgress //
		   && !IsEquipAnimInProgress  //
		   && CurrentWeapon->IsCanReload();
}

void USTUWeaponComponent::OnClipEmpty(ASTUBaseWeapon* AmmoEmptyWeapon)
{
	if (!AmmoEmptyWeapon)
	{
		return;
	}

	if (CurrentWeapon == AmmoEmptyWeapon)
	{
		ChangeClip();
	}
	else
	{
		for (const auto Weapon : Weapons)
		{
			if (Weapon == AmmoEmptyWeapon)
			{
				Weapon->ChangeClip();
			}
		}
	}
}

void USTUWeaponComponent::ChangeClip()
{
	if (!IsCanReload())
	{
		return;
	}

	CurrentWeapon->OnStopFire();
	CurrentWeapon->ChangeClip();
	IsReloadAnimInProgress = true;
	PlayAnimMontage(CurrentReloadAnimMontage);
}

bool USTUWeaponComponent::TryToAddAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType, int32 ClipsAmount)
{
	for (const auto Weapon : Weapons)
	{
		if (Weapon && Weapon->IsA(WeaponType))
		{
			return Weapon->TryToAddAmmo(ClipsAmount);
		}
	}

	return false;
}

bool USTUWeaponComponent::IsNeedAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType)
{
	for (const auto Weapon : Weapons)
	{
		if (Weapon && Weapon->IsA(WeaponType))
		{
			return !Weapon->IsAmmoFull() && Weapon->GetAmmoData().Clips != Weapon->GetDefaultAmmoData().Clips;
		}
	}

	return false;
}

void USTUWeaponComponent::Zoom(bool IsEnabled)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Zoom(IsEnabled);
	}
}