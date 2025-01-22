// Copyright (c) 2025 Xist.GG

#include "MTGSimControlWidget.h"

#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"
#include "MTGPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "MassTimeGame"

void UMTGSimControlWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// by default assume the simulation isn't paused or time dilated (e.g. in design time)
	bool bIsPaused = false;
	float TimeDilation = 1.f;
	UMassSimulationSubsystem* MassSimulationSubsystem {nullptr};

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

		MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
		if (MassSimulationSubsystem)
		{
			bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
			TimeDilation = MassSimulationSubsystem->GetPendingTimeDilationFactor();

			MassSimulationSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);
			MassSimulationSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationPauseStateChanged);

			MassSimulationSubsystem->GetOnTimeDilationChanged().AddUObject(this, &ThisClass::NativeOnSimulationTimeDilationChanged);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("Cannot find MassSimulationSubsystem in World (%s)"), *World->GetName());
		}

		constexpr float bTimerRate = 0.1f;
		constexpr bool bLoopTimer = true;
		World->GetTimerManager().SetTimer(OUT TimerHandle, this, &ThisClass::NativeOnUpdateTimer, bTimerRate, bLoopTimer);
	}

	UpdateWidgetPauseState(bIsPaused);
	UpdateWidgetTimeDilationState(TimeDilation);
	UpdateWidgetTimerState(MassSimulationSubsystem);
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

		UWorld* World = GetWorld();
		check(World);

		if (auto MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulationSubsystem->GetOnSimulationPaused().RemoveAll(this);
			MassSimulationSubsystem->GetOnSimulationResumed().RemoveAll(this);

			MassSimulationSubsystem->GetOnTimeDilationChanged().RemoveAll(this);
		}

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
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	if (SpeedText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 3;

		SpeedText->SetText(FText::AsNumber(TimeDilationFactor, IN &Options));
	}

	if (SpeedDownButton)
	{
		SpeedDownButton->SetIsEnabled(PC->CanDecreaseSimSpeed());
	}

	if (SpeedUpButton)
	{
		SpeedUpButton->SetIsEnabled(PC->CanIncreaseSimSpeed());
	}
}

void UMTGSimControlWidget::UpdateWidgetTimerState(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	const FMassProcessingPhaseManager* PhaseManager = MassSimulationSubsystem ? &MassSimulationSubsystem->GetPhaseManager() : nullptr;

	if (TickNumberText)
	{
		FNumberFormattingOptions Options;
		Options.UseGrouping = true;

		const uint64 TickNumber = PhaseManager ? PhaseManager->GetTickNumber() : 0;
		TickNumberText->SetText(FText::AsNumber(TickNumber, IN &Options));
	}

	if (ElapsedTimeText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 4;

		const double SimTime = PhaseManager ? PhaseManager->GetSimulationTime() : 0;
		ElapsedTimeText->SetText(FText::AsNumber(SimTime, IN &Options));
	}

	if (DeltaTimeText)
	{
		FNumberFormattingOptions Options;
		Options.MinimumIntegralDigits = 1;
		Options.MaximumFractionalDigits = 6;

		const float DeltaTime = PhaseManager ? PhaseManager->GetDeltaTime() : 0;
		DeltaTimeText->SetText(FText::AsNumber(DeltaTime, IN &Options));
	}
}

void UMTGSimControlWidget::NativeOnSimulationPauseStateChanged(UMassSimulationSubsystem* MassSimulationSubsystem)
{
	const bool bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
	UpdateWidgetPauseState(bIsPaused);

	UpdateWidgetTimerState(MassSimulationSubsystem);
}

void UMTGSimControlWidget::NativeOnSimulationTimeDilationChanged(UMassSimulationSubsystem* MassSimulationSubsystem, float NewTimeDilation)
{
	UpdateWidgetTimeDilationState(NewTimeDilation);
}

void UMTGSimControlWidget::NativeOnUpdateTimer()
{
	if (auto MassSimulationSubsystem = GetWorld()->GetSubsystem<UMassSimulationSubsystem>())
	{
		UpdateWidgetTimerState(MassSimulationSubsystem);
	}
}

void UMTGSimControlWidget::NativeOnPauseButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->TogglePlayPause();
}

void UMTGSimControlWidget::NativeOnSpeedDownButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->DecreaseSimSpeed();
}

void UMTGSimControlWidget::NativeOnSpeedUpButtonClicked()
{
	AMTGPlayerController* PC = GetOwningPlayer<AMTGPlayerController>();
	check(PC);

	PC->IncreaseSimSpeed();
}

#undef LOCTEXT_NAMESPACE
