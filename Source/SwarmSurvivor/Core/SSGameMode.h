// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SSGameMode.generated.h"

class ASSSpawnManager;

UCLASS()
class SWARMSURVIVOR_API ASSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASSGameMode();

protected:
	virtual void BeginPlay() override;

	// Spawn Manager
	UPROPERTY(BlueprintReadOnly, Category = "Game Systems")
	TObjectPtr<ASSSpawnManager> SpawnManager;

public:
	// Game state management
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame(bool bPlayerWon);

	// Game timer
	UPROPERTY(BlueprintReadOnly, Category = "Game")
	float GameTime = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	float MaxGameTime = 900.0f;

	// Getter
	UFUNCTION(BlueprintPure, Category = "Game Systems")
	FORCEINLINE ASSSpawnManager* GetSpawnManager() const { return SpawnManager; }

protected:
	virtual void Tick(float DeltaTime) override;

private:
	bool bGameStarted = false;
	bool bGameEnded = false;
};
