// Copyright (c) 2025 Xist.GG

#include "MTGPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MassTimeGame.h"
#include "MTGSimControlWidget.h"
#include "MTGSimTimeSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystemInstanceController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

// Set this to true if you want to see some expanded debug logs to help visualize Niagara Component lifetimes
#define DEBUG_MTG_NIAGARA_SYSTEMS (false && (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT))

// Set Class Defaults
AMTGPlayerController::AMTGPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

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

	const UWorld* World = GetWorld();
	check(World);

	SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>();
	checkf(SimTimeSubsystem, TEXT("MTGSimTimeSubsystem is required"));

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

	SimTimeSubsystem = nullptr;

	Super::EndPlay(EndPlayReason);
}

void AMTGPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Since we're dilating global time, Niagara particles will dilate as well.
	// In the case of the Cursor FX system, we don't want that.
	//
	// Here we will manually tick any Cursor FX particle systems for as long
	// as they live (~0.7s) with the REAL TIME elapsed in the tick, NOT with
	// the dilated game time.

	if (0 < SpawnedFXComponents.Num())
	{
		// Convert the game dilated time to real time
		const float RealDeltaSeconds = SimTimeSubsystem->GetRealTimeSeconds(DeltaSeconds);

		for (auto It = SpawnedFXComponents.CreateIterator(); It; ++It)
		{
			if (UNiagaraComponent* NiagaraComponent = It->Get();
				LIKELY(NiagaraComponent))  // True ~ 41/42 times (0.7s @ 60 fps)
			{
				// The component itself is still valid, try to get the Niagara system instance
				if (FNiagaraSystemInstanceControllerPtr NiagaraController = NiagaraComponent->GetSystemInstanceController();
					LIKELY(NiagaraController))  // True ~ 99.99%
				{
					if (FNiagaraSystemInstance* NiagaraInstance = NiagaraController->GetSoloSystemInstance();
						LIKELY(NiagaraInstance))  // True ~ 99.99%
					{
						NiagaraInstance->SetPaused(false);  // Unpause so we can Advance
						NiagaraInstance->AdvanceSimulation(1, RealDeltaSeconds);
						NiagaraInstance->SetPaused(true);  // Pause so it doesn't tick on its own
						continue;  // DO NOT REMOVE, we're still ticking
					}
				}
			}

			// If we've gotten this far, this niagara component is no longer valid; remove it
			It.RemoveCurrent();

#if DEBUG_MTG_NIAGARA_SYSTEMS
			UE_LOG(LogMassTimeGame, Log, TEXT("Removed expired FXCursor Niagara System Component"));
#endif
		}

#if DEBUG_MTG_NIAGARA_SYSTEMS
		if (0 == SpawnedFXComponents.Num())
		{
			UE_LOG(LogMassTimeGame, Log, TEXT("FXCursor Niagara System Component Set is now empty"));
		}
#endif
	}
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
	const UWorld* World = GetWorld();
	check(World);

	// We track inputs in REAL TIME, not in dilated sim time
	FollowTime += SimTimeSubsystem->GetRealTimeSeconds(World->GetDeltaSeconds());

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

		// Spawn the Niagara cursor component
		UNiagaraComponent* NewFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
		NewFXComponent->SetForceSolo(true);  // Force it into solo mode so we can tick it
		NewFXComponent->SetPaused(true);  // DO NOT let this tick on its own

		// Add the newly spawned Niagara component to the set we will manually tick
		SpawnedFXComponents.Add(NewFXComponent);

#if DEBUG_MTG_NIAGARA_SYSTEMS
		UE_LOG(LogMassTimeGame, Log, TEXT("Created FXCursor Niagara System Component [%s]"), *NewFXComponent->GetName());
#endif
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
	SimTimeSubsystem->TogglePlayPause();
}

void AMTGPlayerController::Input_IncreaseSimSpeed()
{
	SimTimeSubsystem->IncreaseSimSpeed();
}

void AMTGPlayerController::Input_DecreaseSimSpeed()
{
	SimTimeSubsystem->DecreaseSimSpeed();
}

#undef DEBUG_MTG_NIAGARA_SYSTEMS
