// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class FIRSTPERSONSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// called for wasd input
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate); // these Rate s are ratio how strong the game stick leaned.
	void LookUpAtRate(float Rate);
	void MouseTurnAtRate(float Rate); // these Rate s are ratio how strong the game stick leaned.
	void MouseLookUpAtRate(float Rate);

	// Called when Fire
	void FireWeapon();

	bool GetBeamEndLocation( const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	// Set Aiming Action
	void AimingButtonPressed();
	void AimingButtonReleased();

	// Aiming FOV
	float CameraDefaultFOV;
	float CameraZoomedFOV;
	float CameraCurrentFOV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;
	void CameraInterpZoom(float DeltaTime);

	void SetAimMouseRate();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();
	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();
	UFUNCTION()
	void AutoFireReset();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Player Charactor Camera Setup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CharactorCamera;

	// Setup turnrate
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;
	// TurnRate for mouse
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float MouseBaseTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float MouseBaseLookUpRate;
	// TurnRate for Hip
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;
	// TurnRate for Aiming
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float AimTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	float AimLookUpRate;

	// Setup Fire Sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireShotSound;

	// Muzzle Flash Paticle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	// Hip fire montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	// Hit Particle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticle;

	// Beam Particle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticle;
	
	// Is Aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAiming;
    
    // Crosshair
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
    float CrosshairSpreadMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	float ShootTimeDuration;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsFiring;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess = "true"))
	FTimerHandle CrosshairShootTimer;

	bool bFireButtonBressed;

	bool bShouldFire;

	float AutomaticFireRate;

	FTimerHandle AutoFireTimer;
	
public:
	// Get Player Camera Arm
	USpringArmComponent* GetSpringArmComponent() const {return SpringArmComponent;};
	UCameraComponent* GetCharactorCameraComponent() const {return CharactorCamera;};

	// Get Player TurnRate
	float GetPlayerTurnRate() const {return BaseTurnRate;};
	float GetPlayerLookUpRate() const {return BaseLookUpRate;};

	bool GetAiming() const {return bIsAiming;};

	UFUNCTION(BlueprintCallable)
	float GetCrosshairMultiplier() const { return  CrosshairSpreadMultiplier;};
};
