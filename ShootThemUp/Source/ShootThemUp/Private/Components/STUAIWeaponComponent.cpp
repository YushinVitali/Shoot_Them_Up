// Shoot Them Up Game. All Rights Reserved.

#include "Components/STUAIWeaponComponent.h"
#include "Weapon/STUBaseWeapon.h"

void USTUAIWeaponComponent::OnStartFire()
{
	if (!IsCanFire())
	{
		return;
	}

	if (CurrentWeapon->IsAmmoEmpty())
	{
		OnNextWeapon();
	}
	else
	{
		CurrentWeapon->OnStartFire();
	}
}

void USTUAIWeaponComponent::OnNextWeapon()
{
	if (!IsCanEquip())
	{
		return;
	}

	int32 NextIndex{(CurrentWeaponIndex + 1) % Weapons.Num()};
	while (NextIndex != CurrentWeaponIndex)
	{
		if (!Weapons[NextIndex]->IsAmmoEmpty())
		{
			break;
		}
		NextIndex = (NextIndex + 1) % Weapons.Num();
	}

	if (CurrentWeaponIndex != NextIndex)
	{
		CurrentWeaponIndex = NextIndex;
		EquipWeapon(CurrentWeaponIndex);
	}
}
