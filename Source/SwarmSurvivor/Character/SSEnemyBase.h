// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SSEnemyBase.generated.h"

class USphereComponent;
class ASSPlayerCharacter;

UENUM(BlueprintType)
enum class ESSEnemyState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Chasing		UMETA(DisplayName = "Chasing"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Dead		UMETA(DisplayName = "Dead")
};

UCLASS()
class SWARMSURVIVOR_API ASSEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASSEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Detection Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<USphereComponent> DetectionSphere;

	// Enemy Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ChaseSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DetectionRadius = 800.0f;

	// Current State
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	ESSEnemyState CurrentState = ESSEnemyState::Idle;

	// Target reference
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TObjectPtr<ASSPlayerCharacter> TargetPlayer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Health system
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void Die();

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetHealthPercentage() const { return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f; }

	// AI behavior
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void SetState(ESSEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void FindTarget();

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void ChaseTarget();

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void AttackTarget();

protected:
	// Detection events
	UFUNCTION()
	virtual void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// Internal state management
	void UpdateAI(float DeltaTime);
	void UpdateMovement(float DeltaTime);

	// Attack timer
	float LastAttackTime = 0.0f;
	float AttackCooldown = 1.0f;
};
