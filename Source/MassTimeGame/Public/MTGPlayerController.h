// Copyright (c) 2025 Xist.GG

#pragma once

#include "GameFramework/PlayerController.h"
#include "Templates/SubclassOf.h"
#include "MTGPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UMTGSimControlWidget;
class UMTGSimTimeSubsystem;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * MTG Player Controller
 *
 * This is the main player controller used by the project.
 * 
 * This is mostly just the 3rd person player controller, except it adds code that
 * makes the spawned Niagara System Instances immune to time dilation, since they
 * are representative of the player's inputs and movements, which should NOT be
 * time dilated with the rest of the game.
 */
UCLASS()
class AMTGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AMTGPlayerController();

	void Input_TogglePlayPause();

	void Input_IncreaseSimSpeed();
	void Input_DecreaseSimSpeed();

protected:
	/** The class of widget to spawn for the SimControlWidget */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UMTGSimControlWidget> SimControlWidgetClass;

	/** The widget that allows the player to view/control the sim time */
	UPROPERTY()
	TObjectPtr<UMTGSimControlWidget> SimControlWidget;

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UNiagaraSystem> FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	/** Set Destination Action (Mouse) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationClickAction;

	/** Set Destination Action (Touch) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationTouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> TogglePlayPauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> IncreaseSimSpeedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> DecreaseSimSpeedAction;

	//~Begin APlayerController interface
	virtual void SetupInputComponent() override;
	//~End APlayerController interface

	//~Begin AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	//~End AActor interface

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();

private:
	/** Saved reference to MTGSimTimeSubsystem since we need it 1+ times/tick */
	UPROPERTY(Transient)
	TObjectPtr<UMTGSimTimeSubsystem> SimTimeSubsystem;

	/** Set of spawned Niagara Components that we will explicitly tick with real time */
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<UNiagaraComponent>> SpawnedFXComponents;

	FVector CachedDestination;

	bool bIsTouch = false; // Is it a touch device
	float FollowTime; // For how long it has been pressed
};
