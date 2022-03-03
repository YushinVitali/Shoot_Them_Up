// Shoot Them Up Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUCoreTypes.h"
#include "STUHealthComponent.generated.h"

class UCameraShakeBase;
class UPhysicalMaterial;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTTHEMUP_API USTUHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USTUHealthComponent();

	FOnDeathSignature OnDeath;
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return FMath::IsNearlyZero(Health); }

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercent() const { return Health / MaxHealth; };

	float GetHealth() const { return Health; }

	bool TryToAddHealthPercent(float HealthPercent);

	bool IsHealthFull() const;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation,
		class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
		const class UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION()
	void OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin,
		FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	void HealUpdate();

	void SetHealth(float NewHealth);

	void PlayCameraShake();

	void Killed(AController* KillerController);

	void ApplyDamage(float Damage, AController* InstigatedBy);

	float GetPointDamageModifier(AActor* DamageActor, const FName& BoneName);
	
	void ReportDamageEvent(float Damage, AController* InstigatedBy);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxHealth{100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal")
	bool IsAutoHeal{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "IsAutoHeal"))
	float HealthUpdateTime{1.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "IsAutoHeal"))
	float HealthDelay{3.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "IsAutoHeal"))
	float HealthModifier{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heal")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	TMap<UPhysicalMaterial*, float> DamageModifiers;

private:
	FTimerHandle HealTimerHandler;

	float Health{0.0f};
};
