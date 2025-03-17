// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Global.h"
#include "BaseMonster.generated.h"

class AMonsterAIController;
class USphereComponent;
class UAnimMontage;
class UBoxComponent;

UCLASS()
class RPGWORLD_API ABaseMonster : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
					ABaseMonster();

protected:
	virtual void	BeginPlay() override;
	virtual void	Destroyed() override;

public:
	virtual void	Tick(float deltaTime) override;

	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void	Die(AController* damageInstigator) override;

	virtual void	Respawn(float spawnHpRatio = 1.0f) override;

	void			SetSpawnInfo(const FTransform& transform);
	void			SetAIMoveRadius(const float radius) { _aiMoveRadius = radius; }
	void			SetAttackDamage(const int32 attackDamge) { _attackDamage = attackDamge; }
	void			SetMonsterKey(const MonsterKey key) { _monsterKey.SetKeyRawValue(key.GetKey()); }
	void			SetPoolIndex(const int32 index) { _poolIndex = index; }

	FVector			GetSpawnLocation()			const { return _spawnTransform.GetLocation(); }
	float			GetAIMoveRadius()			const { return _aiMoveRadius; }
	MonsterKey		GetMonsterKey()				const { return _monsterKey; }
	int32			GetAttackAnimationCount()	const { return _attackAnimationArray.Num(); }
	int32			GetPoolIndex()				const { return _poolIndex; }

	UFUNCTION(BlueprintImplementableEvent)
	void			PlayAttackAnimation(const int32 index);

	UFUNCTION(BlueprintImplementableEvent)
	void			PlayDeathAnimation(const int32 index);

	UFUNCTION(BlueprintImplementableEvent)
	void			ResetMaterialParmeter();

	UFUNCTION(BlueprintImplementableEvent)
	void			BPRespawn();

	UFUNCTION(BlueprintCallable)
	void			SetAttackCollisionEnabled(bool bEnabled);

private:
	void			ReturnMonsterPool();

	UFUNCTION()
	void			OnRep_SetCollisionEnabled();

	UFUNCTION()
	void			OnOverlapAttackCollision(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
												int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	void			DetectPlayer();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UAnimMontage*>				_attackAnimationArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UAnimMontage*>				_deathAnimationArray;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SetCollisionEnabled)
	bool								_bCollisionEnabled;

	FTimerHandle						_deathTimerHandle;

private:
	UPROPERTY(EditDefaultsOnly)
	UBoxComponent*						_attackCollision;
	
	UPROPERTY(EditDefaultsOnly)
	float								_detectPlayerRadius;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32								_attackDamage;

	TSet<AActor*>						_attackedPlayerSet;

	FTransform							_spawnTransform;

	float								_aiMoveRadius;
	MonsterKey							_monsterKey;
	int32								_poolIndex;
};
