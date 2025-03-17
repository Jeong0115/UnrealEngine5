// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Global.h"
#include "BaseCharacter.generated.h"

UCLASS()
class RPGWORLD_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ABaseCharacter();

protected:
	virtual void	BeginPlay() override;
	
public:
	virtual void	Tick(float DeltaTime) override;
	virtual void	SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	float			TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure)
	int32			GetMaxHealth() const { return _maxHealth; }

	UFUNCTION(BlueprintPure)
	int32			GetCurrentHealth() const { return _currentHealth; }

	UFUNCTION()
	void			SetCurrentHealth(int32 healthValue, AController* damageInstigator);

	virtual void	Die(AController* damageInstigator);
	virtual void	Respawn(float spawnHpRatio);

	bool			IsDead() const { return _bIsDead; }

	// 액터 이동 시, 해당 위치에 다른 액터가 존재하면 근처로 이동
	void			SetActorTransformSafe(const FTransform& transfrom, const float offset = 100.0f, const int32 attemptCount = 8);
	void			SetMaxHealth(const int32 health) { _maxHealth = health; }
	void			Heal(const int32 amount);
	
	UFUNCTION(Server, Reliable)
	void			CheatHeal(const int32 amount);

	UFUNCTION(Server, Reliable)
	void			CheatSetMaxHP(const int32 hp);

private:
	UPROPERTY(EditDefaultsOnly, Replicated)
	int32			_maxHealth;

	UPROPERTY(Replicated)
	int32			_currentHealth;

	UPROPERTY(Replicated)
	bool			_bIsDead;
};