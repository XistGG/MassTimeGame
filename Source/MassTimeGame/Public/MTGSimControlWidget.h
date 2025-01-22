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

	UFUNCTION(BlueprintCallable)
	void UpdateWidgetPauseState(bool bIsPaused);

	UFUNCTION(BlueprintCallable)
	void UpdateWidgetTimeDilationState(float TimeDilationFactor);

	UFUNCTION()
	void NativeOnSimulationPauseStateChanged(UMassSimulationSubsystem* MassSimulationSubsystem);

	UFUNCTION()
	void NativeOnSimulationTimeDilationChanged(UMassSimulationSubsystem* MassSimulationSubsystem, float NewTimeDilation);

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> PauseButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedDownButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SpeedUpButton;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeedText;

	UFUNCTION()
	void NativeOnPauseButtonClicked();

	UFUNCTION()
	void NativeOnSpeedDownButtonClicked();

	UFUNCTION()
	void NativeOnSpeedUpButtonClicked();

};
