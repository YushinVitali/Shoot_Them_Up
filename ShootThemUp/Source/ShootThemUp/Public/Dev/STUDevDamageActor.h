// Shoot Them Up Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUDevDamageActor.generated.h"

class USceneComponent;

UCLASS()
class SHOOTTHEMUP_API ASTUDevDamageActor : public AActor
{
	GENERATED_BODY()

public:
	ASTUDevDamageActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	float Radius{300.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FColor SphereColor{FColor::Red};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Damage{10.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool DoFullDamage{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;
};
