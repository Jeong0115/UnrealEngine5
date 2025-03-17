// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Global.h"
#include "BasePlayer.generated.h"

struct	FInputActionInstance;

class	UBoxComponent;
class	UCameraComponent;
class	UInputAction;
class	UInputComponent;
class	UInputMappingContext;
class	USpringArmComponent;

UENUM(BlueprintType)
enum class ECameraType : uint8
{
	MainCamera,
	VillageCamera
};

UCLASS()
class RPGWORLD_API ABasePlayer : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	ABasePlayer();

protected:
	virtual void	BeginPlay() override;
	virtual void	Destroyed() override;

public:
	virtual void	Tick(float DeltaTime) override;

	virtual void	SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;
	virtual void	PawnClientRestart() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void	Die(AController* damageInstigator) override;
	virtual void	Respawn(float spawnHpRatio) override;

	UFUNCTION()
	void			OnOverlapAttackCollision(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, 
												int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);
	
	UFUNCTION(BlueprintCallable)
	void			ChangeCameraType(const ECameraType type);

	UFUNCTION(Server, Reliable)
	void			AttackActionRPC();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void			ComboAttackActionRPC();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void			ResetComboRPC();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void			HidePlayerAndShowDeathWidget();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void			SetAttackCollisionEnabledRPC(bool bEnabled);

	UFUNCTION(BlueprintNativeEvent)
	void			PlayAttackAnimation(const int32 attackIndex);

	UFUNCTION(NetMulticast, Reliable)
	void			PlayAttackAnimationRPC(const int32 attackIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void			PlayDeathAnimation();

	UFUNCTION(BlueprintCallable)
	bool			IsAutoBattle() const { return _bAutoBattle; }

	UFUNCTION(BlueprintCallable)
	bool			IsAutoPotion() const { return _bAutoPotion; }

	UFUNCTION(BlueprintCallable)
	void			TurnAutoBattle();
	
	UFUNCTION(Client, Reliable)
	void			TurnOffAutoBattle();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void			TurnAutoPotion();

private:
	void			MoveAction(const FInputActionInstance& instance);
	void			LookAction(const FInputActionInstance& instance);
	void			AttackAction();

	void			StartMoveAction()		{ _bMoveInput = true; }
	void			CompleteMoveAction()	{ _bMoveInput = false; }

	void			FindTargetEnemey();
	void			AutoBattle();
	void			TurnToTargetEnemy();
	void			MoveToTargetEnemy();
	void			MoveToStartLocation();

private:
	UPROPERTY(EditDefaultsOnly)
	UBoxComponent*				_attackCollision;

	UPROPERTY(EditDefaultsOnly)
	USpringArmComponent*		_springArm;

	UPROPERTY(EditDefaultsOnly)
	UCameraComponent*			_mainCamera;

	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext*		_playerInputMappingContext;

	UPROPERTY(EditDefaultsOnly)
	UInputAction*				_moveAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction*				_lookAction;

	UPROPERTY(EditDefaultsOnly)
	UInputAction*				_attackAction;

	UPROPERTY(EditDefaultsOnly)
	float						_turnRate;

	UPROPERTY(EditDefaultsOnly)
	int32						_attackDamage;

	UPROPERTY(BlueprintReadOnly, Replicated, meta = (AllowPrivateAccess = "true"))
	bool						_bSaveAttack;

	UPROPERTY(BlueprintReadOnly, Replicated, meta = (AllowPrivateAccess = "true"))
	bool						_isAttacking;

	UPROPERTY(Replicated)
	bool						_bAutoPotion;

	FVector						_autoStartLocation;
	ABaseCharacter*				_targetEnemy;

	float						_attackRange;

	int32						_attackIndex;
	
	ECameraType					_cameraType;

	bool						_bAutoBattle;
	bool						_bMoveInput;
	bool						_isAttackByAutoBattle;
};
