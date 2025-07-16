// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SSSpawnManager.h"
#include "Character/SSEnemyBase.h"
#include "Character/SSPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

// Sets default values
ASSSpawnManager::ASSSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default spawn settings
	SpawnInterval = 2.0f;
	SpawnRadius = 1500.0f;
	MinSpawnDistance = 800.0f;
	MaxEnemies = 50;
	bAutoSpawn = true;

	// Setup default enemy types
	FSSEnemySpawnInfo DefaultEnemyInfo;
	DefaultEnemyInfo.EnemyClass = ASSEnemyBase::StaticClass();
	DefaultEnemyInfo.SpawnWeight = 1.0f;
	DefaultEnemyInfo.MinSpawnTime = 0.0f;
	EnemyTypes.Add(DefaultEnemyInfo);
}

// Called when the game starts or when spawned
void ASSSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Find player character
	PlayerCharacter = Cast<ASSPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Player character not found!"));
	}

	// Record game start time
	GameStartTime = GetWorld()->GetTimeSeconds();
	LastSpawnTime = GameStartTime;

	UE_LOG(LogTemp, Warning, TEXT("SpawnManager initialized - Auto Spawn: %s"), 
		bAutoSpawn ? TEXT("Enabled") : TEXT("Disabled"));
}

// Called every frame
void ASSSpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CleanupDeadEnemies();

	// Auto spawn logic
	if (bAutoSpawn && CanSpawnMoreEnemies())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime >= LastSpawnTime + SpawnInterval)
		{
			SpawnEnemyNearPlayer();
			LastSpawnTime = CurrentTime;
		}
	}
}

ASSEnemyBase* ASSSpawnManager::SpawnEnemy(TSubclassOf<ASSEnemyBase> EnemyClass, const FVector& Location)
{
	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Invalid enemy class!"));
		return nullptr;
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// Spawn the enemy
	ASSEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<ASSEnemyBase>(EnemyClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedEnemy)
	{
		SpawnedEnemies.Add(SpawnedEnemy);
		UE_LOG(LogTemp, Log, TEXT("SpawnManager: Spawned enemy at %s"), *Location.ToString());
		return SpawnedEnemy;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: Failed to spawn enemy at %s"), *Location.ToString());
		return nullptr;
	}
}

void ASSSpawnManager::SpawnEnemyNearPlayer()
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: No player character found for spawning!"));
		return;
	}

	// Get random spawn location
	FVector SpawnLocation = GetRandomSpawnLocation();

	// Select enemy type
	TSubclassOf<ASSEnemyBase> EnemyClass = SelectRandomEnemyType();

	// Spawn the enemy
	SpawnEnemy(EnemyClass, SpawnLocation);
}

void ASSSpawnManager::StartAutoSpawn()
{
	bAutoSpawn = true;
	UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Auto spawn started"));
}

void ASSSpawnManager::StopAutoSpawn()
{
	bAutoSpawn = false;
	UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Auto spawn stopped"));
}

void ASSSpawnManager::ClearAllEnemies()
{
	for (TObjectPtr<ASSEnemyBase> Enemy : SpawnedEnemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}
	SpawnedEnemies.Empty();
	UE_LOG(LogTemp, Warning, TEXT("SpawnManager: All enemies cleared"));
}

int32 ASSSpawnManager::GetAliveEnemyCount() const
{
	int32 AliveCount = 0;
	for (const TObjectPtr<ASSEnemyBase>& Enemy : SpawnedEnemies)
	{
		if (IsValid(Enemy) && Enemy->IsAlive())
		{
			AliveCount++;
		}
	}
	return AliveCount;
}

FVector ASSSpawnManager::GetRandomSpawnLocation() const
{
	if (!PlayerCharacter)
	{
		return GetActorLocation();
	}

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector SpawnLocation;

	// Try to find a valid spawn location
	int32 Attempts = 0;
	const int32 MaxAttempts = 10;

	do
	{
		// Generate random angle
		float RandomAngle = FMath::RandRange(0.0f, 360.0f);

		// Generate random distance between MinSpawnDistance and SpawnRadius
		float RandomDistance = FMath::RandRange(MinSpawnDistance, SpawnRadius);

		// Calculate spawn position
		FVector Direction = FVector(
			FMath::Cos(FMath::RadiansToDegrees(RandomAngle)),
			FMath::Sin(FMath::RadiansToDegrees(RandomAngle)),
			0.0f
		);

		SpawnLocation = PlayerLocation + (Direction * RandomDistance);

		// Try to find navmesh location
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSystem)
		{
			FNavLocation NavLocation;
			if (NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(200.0f, 200.0f, 500.0f)))
			{
				return NavLocation.Location;
			}
		}

		Attempts++;
	} while (Attempts < MaxAttempts);

	// Fallback: spawn at calculated location even if not on navmesh
	UE_LOG(LogTemp, Warning, TEXT("SpawnManager: Could not find navmesh location, using fallback"));
	return SpawnLocation;
}

TSubclassOf<ASSEnemyBase> ASSSpawnManager::SelectRandomEnemyType() const
{
	if (EnemyTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnManager: No enemy types configured!"));
		return ASSEnemyBase::StaticClass();
	}

	// Filter enemy types based on game time
	float CurrentGameTime = GetWorld()->GetTimeSeconds() - GameStartTime;
	TArray<FSSEnemySpawnInfo> ValidEnemyTypes;

	for (const FSSEnemySpawnInfo& EnemyInfo : EnemyTypes)
	{
		if (CurrentGameTime >= EnemyInfo.MinSpawnTime && EnemyInfo.EnemyClass.IsValid())
		{
			ValidEnemyTypes.Add(EnemyInfo);
		}
	}

	if (ValidEnemyTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManager: No valid enemy types for current game time %.1f"), CurrentGameTime);
		return ASSEnemyBase::StaticClass();
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (const FSSEnemySpawnInfo& EnemyInfo : ValidEnemyTypes)
	{
		TotalWeight += EnemyInfo.SpawnWeight;
	}

	// Select random enemy based on weight
	float RandomValue = FMath::RandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (const FSSEnemySpawnInfo& EnemyInfo : ValidEnemyTypes)
	{
		CurrentWeight += EnemyInfo.SpawnWeight;
		if (RandomValue <= CurrentWeight)
		{
			return EnemyInfo.EnemyClass.LoadSynchronous();
		}
	}

	// Fallback to first valid type
	return ValidEnemyTypes[0].EnemyClass.LoadSynchronous();
}

void ASSSpawnManager::CleanupDeadEnemies()
{
	// Remove invalid or dead enemies from the array
	SpawnedEnemies.RemoveAll([](const TObjectPtr<ASSEnemyBase>& Enemy) 
		{
			return (IsValid(Enemy) == false) || (Enemy->IsAlive() == false);
		});
}

