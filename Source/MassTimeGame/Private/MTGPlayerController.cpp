// Copyright (c) 2025 Xist.GG

#include "MTGPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"
#include "Engine/LocalPlayer.h"
#include "MTGSimControlWidget.h"
#include "MTGSimTimeSubsystem.h"

AMTGPlayerController::AMTGPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	ShortPressThreshold = 0.2f;

	// Set default sim control widget
	static ConstructorHelpers::FClassFinder<UMTGSimControlWidget> SimTimeControlBPClass(TEXT("/Game/UI/W_SimTimeControl"));
	if(SimControlWidgetClass == nullptr && SimTimeControlBPClass.Class != nullptr)
	{
		SimControlWidgetClass = SimTimeControlBPClass.Class;
	}
}

void AMTGPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (SimControlWidgetClass)
	{
		// Create the widget AFTER initializing the sim speed settings
		SimControlWidget = CreateWidget<UMTGSimControlWidget>(this, SimControlWidgetClass);
		if (SimControlWidget)
		{
			// Add the widget to the viewport
			SimControlWidget->AddToViewport();
		}
	}

	// Immediately focus the game viewport so key presses go to the game rather than to UEditor
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void AMTGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SimControlWidget)
	{
		SimControlWidget->RemoveFromParent();
		SimControlWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMTGPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AMTGPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AMTGPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AMTGPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AMTGPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AMTGPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AMTGPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AMTGPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AMTGPlayerController::OnTouchReleased);

		// Mass Time inputs
		EnhancedInputComponent->BindAction(TogglePlayPauseAction, ETriggerEvent::Completed, this, &AMTGPlayerController::Input_TogglePlayPause);
		EnhancedInputComponent->BindAction(IncreaseSimSpeedAction, ETriggerEvent::Completed, this, &AMTGPlayerController::Input_IncreaseSimSpeed);
		EnhancedInputComponent->BindAction(DecreaseSimSpeedAction, ETriggerEvent::Completed, this, &AMTGPlayerController::Input_DecreaseSimSpeed);
	}
	else
	{
		UE_LOG(LogMassTimeGame, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMTGPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AMTGPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AMTGPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AMTGPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AMTGPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AMTGPlayerController::Input_TogglePlayPause()
{
	const UWorld* World = GetWorld();
	check(World);

	UMTGSimTimeSubsystem* SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>();
	if (ensureMsgf(SimTimeSubsystem, TEXT("MTGSimTimeSubsystem is required")))
	{
		SimTimeSubsystem->TogglePlayPause();
	}
}

void AMTGPlayerController::Input_IncreaseSimSpeed()
{
	const UWorld* World = GetWorld();
	check(World);

	UMTGSimTimeSubsystem* SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>();
	if (ensureMsgf(SimTimeSubsystem, TEXT("MTGSimTimeSubsystem is required")))
	{
		SimTimeSubsystem->IncreaseSimSpeed();
	}
}

void AMTGPlayerController::Input_DecreaseSimSpeed()
{
	const UWorld* World = GetWorld();
	check(World);

	UMTGSimTimeSubsystem* SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>();
	if (ensureMsgf(SimTimeSubsystem, TEXT("MTGSimTimeSubsystem is required")))
	{
		SimTimeSubsystem->DecreaseSimSpeed();
	}
}
