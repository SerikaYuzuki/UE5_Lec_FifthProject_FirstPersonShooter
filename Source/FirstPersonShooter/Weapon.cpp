// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon()	:
ThrowWeaponTime(0.7f),
bFalling(false)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0};
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw,0.f};
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward {GetItemMesh()->GetForwardVector()};
	const FVector MeshRight {GetItemMesh()->GetRightVector()};
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation {FMath::FRandRange(0,30)};
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0,0,1));
	ImpulseDirection *= 10000;
	GetItemMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}
