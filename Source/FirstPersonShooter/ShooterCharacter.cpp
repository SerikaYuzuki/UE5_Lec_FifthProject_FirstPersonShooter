// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter()	:
CameraDefaultFOV(0),
CameraZoomedFOV(60),
CameraCurrentFOV(0),
ZoomInterpSpeed(20),
BaseTurnRate(45.f),
BaseLookUpRate(45.f),
MouseBaseTurnRate(0.4f),
MouseBaseLookUpRate(0.4f),
bIsAiming(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SpringArm Setup
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;
	SpringArmComponent->bUsePawnControlRotation = true; // Arm rotates relative to arm
	SpringArmComponent->SocketOffset = FVector(0.f, 50.f, 50.f);

	// Camera Setup
	CharactorCamera = CreateDefaultSubobject<UCameraComponent>("CharactorCamera");
	CharactorCamera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CharactorCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Let the controller only affect the camera not Charactor
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure charactor movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0,540,0);
	GetCharacterMovement()->JumpZVelocity = 600;
	GetCharacterMovement()->AirControl = 0.2;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CharactorCamera)
	{
		CameraDefaultFOV = GetCharactorCameraComponent()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
}

// Setup Movement Func
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

// Fire Func
void AShooterCharacter::FireWeapon()
{
	// Fire Sound
	if (FireShotSound)
	{
		UGameplayStatics::PlaySound2D(this, FireShotSound);
	}

	// Socket to Crosshair Position Particle
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("Barrel_Socket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEnd);
			}
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticle, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}

	// Fire Animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

// Muzzle Socket to Hit Location. Return whether hit or not.
bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}
	FVector2D CrosshairLocation(ViewPortSize.X/2.f,ViewPortSize.Y/2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start { CrosshairWorldPosition };
		const FVector End { CrosshairWorldPosition + CrosshairWorldDirection * 50000 };

		OutBeamLocation = End;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;
		}

		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart {MuzzleSocketLocation};
		const FVector WeaponTraceEnd {OutBeamLocation};
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bIsAiming = true;
}
void AShooterCharacter::AimingButtonReleased()
{
	bIsAiming = false;
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bIsAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetCharactorCameraComponent()->SetFieldOfView(CameraCurrentFOV);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// When Aiming Interpo Zoom
	CameraInterpZoom(DeltaTime);

}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// Axis Input Delegate
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("MouseTurn", this, &AShooterCharacter::MouseTurnAtRate);
	PlayerInputComponent->BindAxis("MouseLookUp", this, &AShooterCharacter::MouseLookUpAtRate);

	// Action Input Delegate
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton",IE_Pressed,this,&AShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction("Aiming",IE_Pressed,this,&AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("Aiming",IE_Released,this,&AShooterCharacter::AimingButtonReleased);


}

