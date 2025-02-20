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

	// Override in BP if you want different options
	SimSpeedOptions = {.125f, .25f, .5f, .75f, 1.f, 1.25f, 1.5f, 2.f, 4.f, 8.f};
}

void AMTGPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// By default, set the SimSpeedIndex to whatever index == 1.f
	bool bFound {false};
	for (int32 i = 0; i < SimSpeedOptions.Num(); i++)
	{
		if (SimSpeedOptions[i] == 1.f)
		{
			SimSpeedIndex = i;
			bFound = true;
			break;
		}
	}

	// If there is no 1.f then just get the midpoint of the speed array
	if (!bFound)
	{
		SimSpeedIndex = SimSpeedOptions.Num() / 2;  // int division implicitly floors the result
	}

	// Sanity checks
	check(SimSpeedOptions.Num() > 0);  // must have at least 1 option
	check(FMath::IsWithin(SimSpeedIndex, 0, SimSpeedOptions.Num()));  // valid SimSpeedIndex range

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
		EnhancedInputComponent->BindAction(TogglePlayPauseAction, ETriggerEvent::Completed, this, &AMTGPlayerController::TogglePlayPause);
		EnhancedInputComponent->BindAction(IncreaseSimSpeedAction, ETriggerEvent::Completed, this, &AMTGPlayerController::IncreaseSimSpeed);
		EnhancedInputComponent->BindAction(DecreaseSimSpeedAction, ETriggerEvent::Completed, this, &AMTGPlayerController::DecreaseSimSpeed);
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

void AMTGPlayerController::TogglePlayPause()
{
	if (UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(GetWorld()))
	{
		if (MassSimulationSubsystem->IsSimulationPaused())
		{
			UE_LOG(LogMassTimeGame, Log, TEXT("Resuming Simulation"));
			MassSimulationSubsystem->ResumeSimulation();
		}
		else
		{
			UE_LOG(LogMassTimeGame, Log, TEXT("Pausing Simulation"));
			MassSimulationSubsystem->PauseSimulation();
		}
	}
}

void AMTGPlayerController::IncreaseSimSpeed()
{
	if (false == CanIncreaseSimSpeed())
	{
		// Cannot increase farther
		return;
	}

	if (UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(GetWorld()))
	{
		SimSpeedIndex++;
		const float NewTimeDilationFactor = SimSpeedOptions[SimSpeedIndex];

		//UE_LOG(LogMassTimeGame, Log, TEXT("Increasing Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), NewTimeDilationFactor);
		// TODO
	}
}

void AMTGPlayerController::DecreaseSimSpeed()
{
	if (false == CanDecreaseSimSpeed())
	{
		// Cannot decrease farther
		return;
	}

	if (UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(GetWorld()))
	{
		SimSpeedIndex--;
		const float NewTimeDilationFactor = SimSpeedOptions[SimSpeedIndex];

		//UE_LOG(LogMassTimeGame, Log, TEXT("Decreasing Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), NewTimeDilationFactor);
		// TODO
	}
}
