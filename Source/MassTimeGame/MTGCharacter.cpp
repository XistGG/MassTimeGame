// Copyright (c) 2025 Xist.GG

#include "MTGCharacter.h"

#include "MTGSimTimeSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

// Set Class Defaults
AMTGCharacter::AMTGCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 3000.f;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// This character doesn't need to tick
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AMTGCharacter::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	check(World);

	if (UMTGSimTimeSubsystem* SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>())
	{
		CustomTimeDilation = SimTimeSubsystem->GetRealTimeDilation();

		// Register for OnTimeDilationChanged events
		SimTimeSubsystem->GetOnTimeDilationChanged().AddUObject(this, &ThisClass::NativeOnSimTimeDilationChanged);
	}
}

void AMTGCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		if (UMTGSimTimeSubsystem* SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>())
		{
			// Unregister for OnTimeDilationChanged events
			SimTimeSubsystem->GetOnTimeDilationChanged().RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMTGCharacter::NativeOnSimTimeDilationChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem)
{
	// Sim changed the global time dilation.
	// Adjust this actor's custom time dilation so it runs at real time.
	CustomTimeDilation = SimTimeSubsystem->GetRealTimeDilation();
}
