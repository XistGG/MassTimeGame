// Copyright (c) 2025 Xist.GG

#pragma once

#include "Blueprint/UserWidget.h"
#include "MTGSimControlWidget.generated.h"

class UButton;
class UMTGSimTimeSubsystem;
class UTextBlock;

/**
 * MTG Sim Control Widget
 *
 * This is the widget in the top right of the screen that shows the player
 * the current time, tick number, DeltaTime, etc.
 *
 * This also allows them to click the Pause/Resume button, or the +/- Speed
 * buttons.
 *
 * 100% of the functionality for this widget is implemented in C++ but the
 * actual UI design is done in Blueprint.
 *
 * In order to not be affected by the global time dilation, this widget
 * ticks.  It only updates itself once every WidgetUpdateInterval seconds,
 * which you can configure to your liking.
 */
UCLASS()
class MASSTIMEGAME_API UMTGSimControlWidget
	: public UUserWidget
	, public FTickableGameObject
{
	GENERATED_BODY()

	// Set Class Defaults
	UMTGSimControlWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End UUserWidget interface

	/**
	 * Update the pause state of the widget
	 * @param bIsPaused True if the simulation is currently paused, else False
	 */
	void UpdateWidgetPauseState(bool bIsPaused);

	/**
	 * Update the time dilation state of the widget
	 * @param TimeDilationFactor Current simulation time dilation factor (1 = real time, 0.5 = slow, 2 = fast, ...)
	 */
	void UpdateWidgetTimeDilationState(float TimeDilationFactor);

	/**
	 * Update the time state of the widget
	 *
	 * Time state includes the tick number, elapsed time and the delta time.
	 * This is called VERY OFTEN, based on the value of WidgetUpdateInterval.
	 */
	void UpdateWidgetTimeState();

	/**
	 * Callback from the MTGSimTimeSubsystem when the simulation Pause state changes
	 * @param SimTimeSubsystem Expected to be the same as our cached SimTimeSubsystem
	 */
	void NativeOnSimulationPauseStateChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem);

	/**
	 * Callback from the MTGSimTimeSubsystem when the simulation time dilation changes
	 * @param SimTimeSubsystem Expected to be the same as our cached SimTimeSubsystem
	 */
	void NativeOnSimulationTimeDilationChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem);

	/** Callback when the "Pause/Resume" button is clicked */
	UFUNCTION()
	void NativeOnPauseButtonClicked();

	/** Callback when the "Decrease Sim Speed" button is clicked */
	UFUNCTION()
	void NativeOnSpeedDownButtonClicked();

	/** Callback when the "Increase Sim Speed" button is clicked */
	UFUNCTION()
	void NativeOnSpeedUpButtonClicked();

	/** Persistent reference to the MTGSimTimeSubsystem since we use it 1+ times per tick */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=MassTimeGame)
	TObjectPtr<UMTGSimTimeSubsystem> SimTimeSubsystem;

	/** Real Time (seconds) between updates of the widget values */
	UPROPERTY(EditAnywhere, Category=MassTimeGame)
	float WidgetUpdateInterval;

	/** The Pause/Resume button. Its text is updated dynamically based on the sim state. */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> PauseButton;

	/** The "decrease sim speed" button */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedDownButton;

	/** The "increase sim speed" button */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedUpButton;

	/** A text block to display the current Speed Factor */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeedText;

	/** A text block to display the simulation tick number */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TickNumberText;

	/** A text block to display the simulation elapsed time (in sim time seconds) */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ElapsedTimeText;

	/** A text block to display the current simulation Delta Time (in real seconds) */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DeltaTimeText;

private:
	/** How long it has been (real time seconds) since we last updated the widget */
	float TimeSinceLastUpdate = MAX_flt / 2.;  // A huge number

public:
	//~Begin FTickableGameObject interface
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMTGSimControlWidget, STATGROUP_Tickables); }
	//~End FTickableGameObject interface
};
