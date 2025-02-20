// Copyright (c) 2025 Xist.GG

#pragma once

#include "Blueprint/UserWidget.h"
#include "MTGSimControlWidget.generated.h"

class UButton;
class UMassSimulationSubsystem;
class UTextBlock;

UCLASS()
class MASSTIMEGAME_API UMTGSimControlWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void UpdateWidgetPauseState(bool bIsPaused);

	void UpdateWidgetTimeDilationState(float TimeDilationFactor);

	void UpdateWidgetTimerState(UMassSimulationSubsystem* MassSimulationSubsystem);

	void NativeOnSimulationPauseStateChanged(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);

	UFUNCTION()
	void NativeOnUpdateTimer();

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> PauseButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedDownButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedUpButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeedText;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TickNumberText;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ElapsedTimeText;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> DeltaTimeText;

	UFUNCTION()
	void NativeOnPauseButtonClicked();

	UFUNCTION()
	void NativeOnSpeedDownButtonClicked();

	UFUNCTION()
	void NativeOnSpeedUpButtonClicked();

private:
	FTimerHandle TimerHandle;

};
