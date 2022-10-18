// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "Item.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
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
HipTurnRate(0.4f),
HipLookUpRate(0.4f),
AimTurnRate(0.3f),
AimLookUpRate(0.3f),
bIsAiming(false),
CrosshairSpreadMultiplier(0.f),
CrosshairVelocityFactor(0.f),
CrosshairInAirFactor(0.f),
CrosshairAimFactor(0.f),
CrosshairShootingFactor(0.f),
ShootTimeDuration(0.5f),
bIsFiring(false),
bFireButtonBressed(false),
bShouldFire(true),
AutomaticFireRate(0.1),
bShoudTraceForItems(false)
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

	EquipWeapon(SpawnDefaultWeapon());
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

	// Start Bullet fire timer for crosshair
	StartCrosshairBulletFire();
}
void AShooterCharacter::FireButtonPressed()
{
	bFireButtonBressed = true;
	StartFireTimer();
}
void AShooterCharacter::FireButtonReleased()
{
	bFireButtonBressed = false;
}
void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}
void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonBressed)
	{
		StartFireTimer();
	}
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}
	FVector2D CrosshairLocation(ViewPortSize.X/2.f,ViewPortSize.Y/2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start {CrosshairWorldPosition};
		FVector End { Start + CrosshairWorldDirection * 5000.f };
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End,ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
	
}
void AShooterCharacter::TraceForItems()
{
	if (bShoudTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshair(ItemTraceResult, HitLocation);

		if (ItemTraceResult.bBlockingHit)
		{

			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (HitItem && HitItem->GetPickupWidget())
			{
				HitItem->GetPickupWidget()->SetVisibility(true);
			}

			if (TraceHitItemLastFrame)
			{
				if (HitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			TraceHitItemLastFrame = HitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

// Muzzle Socket to Hit Location. Return whether hit or not.
bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else
	{
		
	}
	
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart {MuzzleSocketLocation};
	const FVector NormBeam {(OutBeamLocation - MuzzleSocketLocation).GetSafeNormal(1)};
	const FVector WeaponTraceEnd {MuzzleSocketLocation + NormBeam *5000.f};
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
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
void AShooterCharacter::SetAimMouseRate()
{
	
	if (bIsAiming)
	{
		BaseTurnRate = AimTurnRate;
		BaseLookUpRate = AimLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

// Crosshair HUD
void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f,600.f};
	FVector2D VelocityMultiplierRange { 0.f, 1.f};
	FVector Velocity {GetVelocity()};
	Velocity.Z = 0;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25, DeltaTime, 2.25);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0, DeltaTime, 13);
	}
	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.3, DeltaTime, 2.25);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 13);
	}
	if (bIsFiring)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3, DeltaTime, 15);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0, DeltaTime, 15);
	}
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor +CrosshairAimFactor + CrosshairShootingFactor;
}
void AShooterCharacter::StartCrosshairBulletFire()
{
	bIsFiring = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}
void AShooterCharacter::FinishCrosshairBulletFire()
{
	bIsFiring = false;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}
void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		WeaponToEquip->GetAreaSphere()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponToEquip->GetCollisionBox()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	DropWeapon();
}

void AShooterCharacter::SelectButtonReleased()
{
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// When Aiming Interpo Zoom
	CameraInterpZoom(DeltaTime);

	SetAimMouseRate();

	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();
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
	PlayerInputComponent->BindAction("FireButton",IE_Pressed,this,&AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton",IE_Released,this,&AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Aiming",IE_Pressed,this,&AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("Aiming",IE_Released,this,&AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Select",IE_Pressed,this,&AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select",IE_Released,this,&AShooterCharacter::SelectButtonReleased);

}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemsCount + Amount <= 0)
	{
		OverlappedItemsCount = 0;
		bShoudTraceForItems = false;
	}
	else
	{
		OverlappedItemsCount += Amount;
		bShoudTraceForItems = true;
	}
}

