// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter()	:
BaseTurnRate(45.f),
BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SpringArm Setup
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;
	SpringArmComponent->bUsePawnControlRotation = true; // Arm rotates relative to arm

	// Camera Setup
	CharactorCamera = CreateDefaultSubobject<UCameraComponent>("CharactorCamera");
	CharactorCamera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CharactorCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && (Value != 0))
	{
		FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation{0,Rotation.Yaw,0};

		FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && (Value != 0))
	{
		FRotator Rotation{Controller->GetControlRotation()};
		FRotator YawRotation{0,Rotation.Yaw,0};

		FVector Direction {FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

/*
 These are Camera Control Functions
 */
void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}
void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(-Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
void AShooterCharacter::MouseTurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * MouseBaseTurnRate);
}
void AShooterCharacter::MouseLookUpAtRate(float Rate)
{
	AddControllerPitchInput(-Rate * MouseBaseLookUpRate);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::MouseTurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::MouseLookUpAtRate);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);
}

