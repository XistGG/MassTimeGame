// Copyright (c) 2025 Xist.GG

#pragma once

#include "Blueprint/UserWidget.h"
#include "MTGSimControlWidget.generated.h"

class UButton;
class UMTGSimTimeSubsystem;
class UTextBlock;

UCLASS()
class MASSTIMEGAME_API UMTGSimControlWidget
	: public UUserWidget
	, public FTickableGameObject
{
	GENERATED_BODY()

	// Set Class Defaults
	UMTGSimControlWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void UpdateWidgetPauseState(bool bIsPaused);

	void UpdateWidgetTimeDilationState(float TimeDilationFactor);

	void UpdateWidgetTimerState();

	void NativeOnSimulationPauseStateChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem);
	void NativeOnSimulationTimeDilationChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category=MassTimeGame)
	TObjectPtr<UMTGSimTimeSubsystem> SimTimeSubsystem;

	UPROPERTY(EditAnywhere, Category=MassTimeGame)
	float WidgetUpdateInterval;

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
	float TimeSinceLastUpdate = MAX_flt / 2.;  // A huge number

public:
	//~Begin FTickableGameObject interface
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMTGSimControlWidget, STATGROUP_Tickables); }
	//~End FTickableGameObject interface
};
