// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "Net\UnrealNetwork.h"
#include "Engine\Engine.h"

ABaseCharacter::ABaseCharacter()
	: _bIsDead(false)
{
	PrimaryActorTick.bCanEverTick = true;
	_maxHealth = 20;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	_currentHealth = _maxHealth;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, _currentHealth);
	DOREPLIFETIME(ABaseCharacter, _maxHealth);
	DOREPLIFETIME(ABaseCharacter, _bIsDead);
}

float ABaseCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	int32 damageApplied = _currentHealth - DamageTaken;

	SetCurrentHealth(damageApplied, EventInstigator);
	return DamageTaken;
}

void ABaseCharacter::Respawn(float spawnHpRatio)
{
	ensureMsgfDebug(spawnHpRatio >= 0.f && spawnHpRatio <= 1.0f, TEXT("비정상적인 spawnHpRatio : [%f] 입니다."), spawnHpRatio);

	if (HasAuthority() == false)
	{
		return;
	}

	if (_bIsDead == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("캐릭터가 죽지 않았는데, Respawn() 이 호출되었습니다."));
		return;
	}

	_bIsDead = false;
	
	_currentHealth = FMath::Clamp(static_cast<int32>(_maxHealth * spawnHpRatio), 0, _maxHealth);
}

void ABaseCharacter::SetActorTransformSafe(const FTransform& transfrom, const float offset, const int32 attemptCount)
{
	FTransform attemptTransform = transfrom;

	for (int32 i = 0; i < attemptCount; ++i)
	{
		FHitResult hitResult;

		bool bMoved = SetActorTransform(attemptTransform, true, &hitResult, ETeleportType::TeleportPhysics);
		if (bMoved == false || hitResult.IsValidBlockingHit())
		{
			FVector offsetDirection = hitResult.ImpactNormal;
			if (i > 0)
			{
				float angle = i * 45.f;
				offsetDirection = offsetDirection.RotateAngleAxis(angle, FVector::UpVector);
				offsetDirection.Normalize();
			}

			FVector location = hitResult.ImpactPoint + offsetDirection * offset;
			attemptTransform.SetLocation(location);
		}
		else
		{
			break;
		}
	}
	return;
}

void ABaseCharacter::Heal(const int32 amount)
{
	ensureMsgfDebug(amount >= 0, TEXT("HealAmount가 음수입니다 : [%d]"), amount);

	if (HasAuthority() == false)
	{
		return;
	}

	_currentHealth = FMath::Clamp(_currentHealth + amount, _currentHealth, _maxHealth);
}

void ABaseCharacter::SetCurrentHealth(int32 healthValue, AController* damageInstigator)
{
	if (HasAuthority() == false)
	{
		return;
	}

	_currentHealth = FMath::Clamp(healthValue, 0, _maxHealth);

	if (_currentHealth <= 0)
	{
		Die(damageInstigator);
	}
}

void ABaseCharacter::Die(AController* damageInstigator)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (_bIsDead == true)
	{
		return;
	}

	_bIsDead = true;
}


void ABaseCharacter::CheatHeal_Implementation(const int32 amount)
{
	Heal(amount);
}

void ABaseCharacter::CheatSetMaxHP_Implementation(const int32 hp)
{
	SetMaxHealth(hp);
}
