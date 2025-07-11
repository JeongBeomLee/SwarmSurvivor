// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SSGameMode.h"
#include "Character/SSPlayerCharacter.h"
//#include "Engine/Engine.h"
//#include "Engine/World.h"
//#include "TimerManager.h"

ASSGameMode::ASSGameMode()
{
	// TODO: Set default pawn class to our custom player character
	// This will be set later when we create the player character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SwarmSurvivor/Character/BP_SSPlayerCharacter"));
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Script/Engine.Blueprint'/Game/SwarmSurvivor/Character/BP_SSPlayerCharacter.BP_SSPlayerCharacter'"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		// Fallback to C++ class if Blueprint is not found
		DefaultPawnClass = ASSPlayerCharacter::StaticClass();
		UE_LOG(LogTemp, Warning, TEXT("BP_SSPlayerCharacter not found! Using C++ class as fallback."));
	}

	// Enable tick for game timer
	PrimaryActorTick.bCanEverTick = true;

	// Default game settings
	MaxGameTime = 900.0f;
}

void ASSGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize game state
	GameTime = 0.0f;
	bGameStarted = false;
	bGameEnded = false;

	// Start the game after a short delay
	FTimerHandle StartGameTimer;
	GetWorldTimerManager().SetTimer(StartGameTimer, this, &ASSGameMode::StartGame, 1.0f, false);
}

void ASSGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGameStarted == true && bGameEnded == false)
	{
		GameTime += DeltaTime;

		if (GameTime >= MaxGameTime)
		{
			EndGame(true);
		}
	}
}

void ASSGameMode::StartGame()
{
	if (bGameStarted == false)
	{
		bGameStarted = true;

		UE_LOG(LogTemp, Warning, TEXT("Game Started!"));

		// TODO: Initialize wave manager, spawn manager, etc.
		// This will be implemented in later days
	}
}

void ASSGameMode::EndGame(bool bPlayerWon)
{
	if (bGameEnded == false)
	{
		bGameEnded = true;

		if (bPlayerWon == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Won! Game Time: %.2f"), GameTime);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Lost! Game Time: %.2f"), GameTime);
		}

		// TODO: Show game over screen, save statistics, etc.
		// This will be implemented in later days
	}
}
