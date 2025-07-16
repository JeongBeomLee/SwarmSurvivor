// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SSSpawnManager.generated.h"

class ASSEnemyBase;
class ASSPlayerCharacter;

USTRUCT(BlueprintType)
struct SWARMSURVIVOR_API FSSEnemySpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSoftClassPtr<ASSEnemyBase> EnemyClass;

	// Spawn weight (higher = more likely to spawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnWeight = 1.0f;
	
	// Minimum time before this enemy can spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MinSpawnTime = 0.0f;

	FSSEnemySpawnInfo()
	{
		SpawnWeight = 1.0f;
		MinSpawnTime = 0.0f;
	}
};

UCLASS()
class SWARMSURVIVOR_API ASSSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASSSpawnManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Spawn settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float SpawnInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float SpawnRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float MinSpawnDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 MaxEnemies = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	bool bAutoSpawn = true;

	// Enemy types to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Types")
	TArray<FSSEnemySpawnInfo> EnemyTypes;

	// Current spawned enemies
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TArray<TObjectPtr<ASSEnemyBase>> SpawnedEnemies;

	// Player reference
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<ASSPlayerCharacter> PlayerCharacter;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Manual spawn functions
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	ASSEnemyBase* SpawnEnemy(TSubclassOf<ASSEnemyBase> EnemyClass, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnEnemyNearPlayer();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void StartAutoSpawn();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void StopAutoSpawn();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ClearAllEnemies();

	// Getter
	UFUNCTION(BlueprintPure, Category = "Spawn")
	int32 GetAliveEnemyCount() const;

	UFUNCTION(BlueprintPure, Category = "Spawn")
	FORCEINLINE bool CanSpawnMoreEnemies() const { return GetAliveEnemyCount() < MaxEnemies; }

protected:
	// Internal spawn logic
	FVector GetRandomSpawnLocation() const;
	TSubclassOf<ASSEnemyBase> SelectRandomEnemyType() const;
	void CleanupDeadEnemies();

private:
	// Spawn timer
	float LastSpawnTime = 0.0f;

	// Game time tracking
	float GameStartTime = 0.0f;
};
