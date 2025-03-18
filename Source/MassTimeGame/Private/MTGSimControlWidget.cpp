// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassTimeGame.h"
#include "MTGSimTimeSubsystem.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "MassTimeGame"

// Set Class Defaults
UMTGSimControlWidget::UMTGSimControlWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetUpdateInterval = 0.05f;
}

void UMTGSimControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// by default assume the simulation isn't paused or time dilated (e.g. in design time)
	bool bIsPaused = false;
	float TimeDilation = 1.f;

	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnPauseButtonClicked);
			ensureAlwaysMsgf(!PauseButton->GetIsFocusable(), TEXT("PauseButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		if (SpeedDownButton)
		{
			SpeedDownButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnSpeedDownButtonClicked);
			ensureAlwaysMsgf(!SpeedDownButton->GetIsFocusable(), TEXT("SpeedDownButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		if (SpeedUpButton)
		{
			SpeedUpButton->OnClicked.AddDynamic(this, &ThisClass::NativeOnSpeedUpButtonClicked);
			ensureAlwaysMsgf(!SpeedUpButton->GetIsFocusable(), TEXT("SpeedUpButton should be set as non-focusable for SPACEBAR to always go to the PlayerController"));
		}

		UWorld* World = GetWorld();
		check(World);

		SimTimeSubsystem = World->GetSubsystem<UMTGSimTimeSubsystem>();
		if (ensureAlwaysMsgf(SimTimeSubsystem, TEXT("MTGSimTimeSubsystem is required")))
		{
			bIsPaused = SimTimeSubsystem->IsPaused();
			TimeDilation = SimTimeSubsystem->GetSimTimeDilation();

			SimTimeSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			SimTimeSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			SimTimeSubsystem->GetOnTimeDilationChanged().AddUObject(this, &UMTGSimControlWidget::NativeOnSimulationTimeDilationChanged);
		}
	}

	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimeDilationState(TimeDilation);
	UpdateWidgetTimeState();
}

void UMTGSimControlWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		if (PauseButton)
		{
			PauseButton->OnClicked.RemoveAll(this);
		}

		if (SpeedDownButton)
		{
			SpeedDownButton->OnClicked.RemoveAll(this);
		}

		if (SpeedUpButton)
		{
			SpeedUpButton->OnClicked.RemoveAll(this);
		}

		if (SimTimeSubsystem)
		{
			SimTimeSubsystem->GetOnSimulationPaused().RemoveAll(this);
			SimTimeSubsystem->GetOnSimulationResumed().RemoveAll(this);
			SimTimeSubsystem->GetOnTimeDilationChanged().RemoveAll(this);
			SimTimeSubsystem = nullptr;
		}
	}

	Super::NativeDestruct();
}

void UMTGSimControlWidget::UpdateWidgetPauseState(bool bIsPaused)
{
	if (PauseButton)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(PauseButton->GetContent()))
		{
			const FText NewText = bIsPaused
				? LOCTEXT("ResumeButtonText", "Resume")
				: LOCTEXT("PauseButtonText", "Pause");

			TextBlock->SetText(NewText);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("PauseButton does not have a TextBlock child as its content widget"));
		}
	}
}

void UMTGSimControlWidget::UpdateWidgetTimeDilationState(float TimeDilationFactor)
{
	if (SpeedText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 3;

		SpeedText->SetText(FText::AsNumber(TimeDilationFactor, IN &Options));
	}

	if (SpeedDownButton)
	{
		SpeedDownButton->SetIsEnabled(SimTimeSubsystem && SimTimeSubsystem->CanDecreaseSimSpeed());
	}

	if (SpeedUpButton)
	{
		SpeedUpButton->SetIsEnabled(SimTimeSubsystem && SimTimeSubsystem->CanIncreaseSimSpeed());
	}
}

void UMTGSimControlWidget::UpdateWidgetTimeState()
{
	uint64 SimTickNumber {0};
	double SimTime {0.};
	float SimDeltaTime {0.};

	if (LIKELY(SimTimeSubsystem))
	{
		SimTickNumber = SimTimeSubsystem->GetSimTickNumber();
		SimTime = SimTimeSubsystem->GetSimTimeElapsed();
		SimDeltaTime = SimTimeSubsystem->GetSimDeltaTime();
	}

	if (TickNumberText)
	{
		FNumberFormattingOptions Options;
		Options.UseGrouping = true;

		TickNumberText->SetText(FText::AsNumber(SimTickNumber, IN &Options));
	}

	if (ElapsedTimeText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 4;

		ElapsedTimeText->SetText(FText::AsNumber(SimTime, IN &Options));
	}

	if (DeltaTimeText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 6;

		DeltaTimeText->SetText(FText::AsNumber(SimDeltaTime, IN &Options));
	}
}

void UMTGSimControlWidget::NativeOnSimulationPauseStateChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystemIn)
{
	checkf(SimTimeSubsystem == SimTimeSubsystemIn, TEXT("We should never receive this event except from our expected SimTimeSubsystem"));
	const bool bIsPaused = SimTimeSubsystem->IsPaused();
	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimeState();
}

void UMTGSimControlWidget::NativeOnSimulationTimeDilationChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystemIn)
{
	checkf(SimTimeSubsystem == SimTimeSubsystemIn, TEXT("We should never receive this event except from our expected SimTimeSubsystem"));
	const float TimeDilation = SimTimeSubsystem->GetSimTimeDilation();
	UpdateWidgetTimeDilationState(TimeDilation);
}

void UMTGSimControlWidget::NativeOnPauseButtonClicked()
{
	if (SimTimeSubsystem)
	{
		SimTimeSubsystem->TogglePlayPause();
	}
}

void UMTGSimControlWidget::NativeOnSpeedDownButtonClicked()
{
	if (SimTimeSubsystem)
	{
		SimTimeSubsystem->DecreaseSimSpeed();
	}
}

void UMTGSimControlWidget::NativeOnSpeedUpButtonClicked()
{
	if (SimTimeSubsystem)
	{
		SimTimeSubsystem->IncreaseSimSpeed();
	}
}

UWorld* UMTGSimControlWidget::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

ETickableTickType UMTGSimControlWidget::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

void UMTGSimControlWidget::Tick(float DeltaTime)
{
	if (LIKELY(SimTimeSubsystem))
	{
		// When we're dilating the global time, that means World Timers are also dilated,
		// which means at really slow time dilation this widget will almost never update!
		// This means we need to tick every frame, and keep track of the ACTUAL REAL TIME
		// that has elapsed since our last update, and update when needed.

		const float RealDeltaTime = SimTimeSubsystem->GetRealTimeSeconds(DeltaTime);
		TimeSinceLastUpdate += RealDeltaTime;

		if (TimeSinceLastUpdate >= WidgetUpdateInterval)
		{
			TimeSinceLastUpdate = 0.;
			UpdateWidgetTimeState();
		}
	}
}

#undef LOCTEXT_NAMESPACE
