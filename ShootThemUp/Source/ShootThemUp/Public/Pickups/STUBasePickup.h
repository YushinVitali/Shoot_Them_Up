// Shoot Them Up Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUBasePickup.generated.h"

class USphereComponent;
class USoundCue;

UCLASS()
class SHOOTTHEMUP_API ASTUBasePickup : public AActor
{
	GENERATED_BODY()

public:
	ASTUBasePickup();

protected:
	virtual void BeginPlay() override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual bool GivePickupTo(APawn* PlayerPawn);

public:
	virtual void Tick(float DeltaTime) override;

	bool IsCouldBeTaken() const;

private:
	void PickupWasTaken();
	void Respawn();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool IsRotation{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool IsBouncing{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float RespawnTime{5.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (EditCondition = "IsRotation"))
	FRotator Rotating{0.0f, 0.0f, 0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (EditCondition = "IsBouncing"))
	float BouncingAmplitude{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (EditCondition = "IsBouncing"))
	float BouncingFrequency{0.0f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* PickupTakenSound;

private:
	FVector InitialLocation;
	FTimerHandle RespawnTimerHandle;
};
