// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SSEnemyBase.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SSPlayerCharacter.h"

// Sets default values
ASSEnemyBase::ASSEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind overlap events
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASSEnemyBase::OnDetectionSphereBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ASSEnemyBase::OnDetectionSphereEndOverlap);

	// Configure character movement
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	CharacterMovementComponent->MaxWalkSpeed = ChaseSpeed;
	CharacterMovementComponent->BrakingDecelerationWalking = 2000.0f;

	// Don't rotate with controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set default stats
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	AttackDamage = 20.0f;
	AttackRange = 100.0f;
	ChaseSpeed = 300.0f;
	DetectionRadius = 800.0f;
	AttackCooldown = 1.0f;

	// Set capsule collision
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// Set default mesh for testing
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		// Make enemy visually different (red tint)
		GetMesh()->SetRenderCustomDepth(true);
	}
}

// Called when the game starts or when spawned
void ASSEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;

	// Set initial state
	SetState(ESSEnemyState::Idle);

	// Update detection sphere radius
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(DetectionRadius);
	}

	// Update movement speed
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->MaxWalkSpeed = ChaseSpeed;
	}

	// Find initial target
	FindTarget();

	UE_LOG(LogTemp, Warning, TEXT("Enemy spawned: %s"), *GetName());
}

// Called every frame
void ASSEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive())
	{
		UpdateAI(DeltaTime);
		UpdateMovement(DeltaTime);
	}
}

void ASSEnemyBase::TakeDamage(float DamageAmount)
{
	if (IsAlive() == false) return;

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

	UE_LOG(LogTemp, Warning, TEXT("%s took %.1f damage, Health: %.1f / %.1f"),
		*GetName(), DamageAmount, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ASSEnemyBase::Die()
{
	SetState(ESSEnemyState::Dead);

	// Stop movement
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->StopMovementImmediately();
		CharacterMovementComponent->DisableMovement();
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UE_LOG(LogTemp, Warning, TEXT("%s died!"), *GetName());

	// TODO: Drop experience points, play death effects
	// This will be implemented in later days

	// Destroy after delay
	SetLifeSpan(2.0f);
}

void ASSEnemyBase::SetState(ESSEnemyState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;

		switch (CurrentState)
		{
		case ESSEnemyState::Idle:
			GetCharacterMovement()->StopMovementImmediately();
			break;
		case ESSEnemyState::Chasing:
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
			break;
		case ESSEnemyState::Attacking:
			GetCharacterMovement()->StopMovementImmediately();
			break;
		case ESSEnemyState::Dead:
			GetCharacterMovement()->DisableMovement();
			break;
		}

		if (SS_DEBUG)
		{
			UE_LOG(LogTemp, Log, TEXT("%s state changed to %d"), *GetName(), (int32)CurrentState);
		}
	}
}

void ASSEnemyBase::FindTarget()
{
	if (!TargetPlayer)
	{
		TargetPlayer = Cast<ASSPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	if (TargetPlayer && CurrentState == ESSEnemyState::Idle)
	{
		float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
		if (DistanceToPlayer <= DetectionRadius)
		{
			SetState(ESSEnemyState::Chasing);
		}
	}
}

void ASSEnemyBase::ChaseTarget()
{
	if (!TargetPlayer || !IsAlive()) return;

	float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

	if (DistanceToTarget <= AttackRange)
	{
		SetState(ESSEnemyState::Attacking);
		return;
	}

	if (DistanceToTarget > DetectionRadius * 1.2f)
	{
		SetState(ESSEnemyState::Idle);
		TargetPlayer = nullptr;
		return;
	}

	// Move towards target
	FVector DirectionToTarget = (TargetPlayer->GetActorLocation(), -GetActorLocation()).GetSafeNormal();
	AddMovementInput(DirectionToTarget, 1.0f);
}

void ASSEnemyBase::AttackTarget()
{
	if (!TargetPlayer || !IsAlive()) return;

	float DistanceToTarget = FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());

	if (DistanceToTarget > AttackRange)
	{
		SetState(ESSEnemyState::Chasing);
		return;
	}

	// Perform attack if cooldown is ready
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime >= LastAttackTime + AttackCooldown)
	{
		LastAttackTime = CurrentTime;

		// TODO: Implement actual attack logic (damage player)
		// This will be implemented when we add damage system
		UE_LOG(LogTemp, Warning, TEXT("%s attacks player for %.1f damage!"), *GetName(), AttackDamage);

		// Later we'll add: TargetPlayer->TakeDamage(AttackDamage);
	}
}

void ASSEnemyBase::OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASSPlayerCharacter* Player = Cast<ASSPlayerCharacter>(OtherActor))
	{
		TargetPlayer = Player;
		if (CurrentState == ESSEnemyState::Idle)
		{
			SetState(ESSEnemyState::Chasing);
		}

		UE_LOG(LogTemp, Log, TEXT("%s detected player"), *GetName());
	}
}

void ASSEnemyBase::OnDetectionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ASSPlayerCharacter* Player = Cast<ASSPlayerCharacter>(OtherActor))
	{
		// Don't immediately lose target, let the distance check in ChaseTarget handle it
		UE_LOG(LogTemp, Log, TEXT("%s lost player from detection sphere"), *GetName());
	}
}

void ASSEnemyBase::UpdateAI(float DeltaTime)
{
	switch (CurrentState)
	{
	case ESSEnemyState::Idle:
		FindTarget();
		break;
	case ESSEnemyState::Chasing:
		ChaseTarget();
		break;
	case ESSEnemyState::Attacking:
		AttackTarget();
		break;
	case ESSEnemyState::Dead:
		// Do nothing when dead
		break;
	}
}

void ASSEnemyBase::UpdateMovement(float DeltaTime)
{
	// TODO: Additional movement logic can be added here if needed
	// For now, the basic movement is handled by CharacterMovementComponent
}

