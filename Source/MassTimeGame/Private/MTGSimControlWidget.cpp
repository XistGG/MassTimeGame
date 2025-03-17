// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassTimeGame.h"
#include "MTGSimTimeSubsystem.h"
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
			TimeDilation = SimTimeSubsystem->GetTimeDilation();

			SimTimeSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			SimTimeSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
		}

		constexpr bool bLoopTimer = true;
		World->GetTimerManager().SetTimer(OUT TimerHandle, this, &ThisClass::NativeOnUpdateTimer, WidgetUpdateInterval, bLoopTimer);
	}

	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimeDilationState(TimeDilation);
	UpdateWidgetTimerState();
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
		}

		UWorld* World = GetWorld();
		check(World);

		World->GetTimerManager().ClearTimer(TimerHandle);
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

void UMTGSimControlWidget::UpdateWidgetTimerState()
{
	uint64 SimTickNumber {0};
	double SimTime {0.};
	float SimDeltaTime {0.};

	if (SimTimeSubsystem)
	{
		SimTickNumber = SimTimeSubsystem->GetTickNumber();
		SimTime = SimTimeSubsystem->GetTime();
		SimDeltaTime = SimTimeSubsystem->GetDeltaTime();
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

void UMTGSimControlWidget::NativeOnSimulationPauseStateChanged(TNotNull<UMTGSimTimeSubsystem*> SimTimeSubsystem)
{
	const bool bIsPaused = SimTimeSubsystem->IsPaused();
	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimerState();
}

void UMTGSimControlWidget::NativeOnUpdateTimer()
{
	UpdateWidgetTimerState();
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

#undef LOCTEXT_NAMESPACE
