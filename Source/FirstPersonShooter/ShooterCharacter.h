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

	
public:
	// Get Player Camera Arm
	USpringArmComponent* GetSpringArmComponent() const {return SpringArmComponent;};
	UCameraComponent* GetCharactorCameraComponent() const {return CharactorCamera;};

	// Get Player TurnRate
	float GetPlayerTurnRate() const {return BaseTurnRate;};
	float GetPlayerLookUpRate() const {return BaseLookUpRate;};
};