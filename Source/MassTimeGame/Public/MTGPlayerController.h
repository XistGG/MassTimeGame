// Copyright (c) 2025 Xist.GG

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "MTGPlayerController.generated.h"

class UMTGSimControlWidget;
/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;

UCLASS()
class AMTGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMTGPlayerController();

	void TogglePlayPause();

	bool CanIncreaseSimSpeed() const { return SimSpeedIndex < SimSpeedOptions.Num() - 1; }
	bool CanDecreaseSimSpeed() const { return SimSpeedIndex > 0; }

	void IncreaseSimSpeed();
	void DecreaseSimSpeed();

protected:
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UMTGSimControlWidget> SimControlWidgetClass;

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
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationClickAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> SetDestinationTouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> TogglePlayPauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> IncreaseSimSpeedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	TObjectPtr<UInputAction> DecreaseSimSpeedAction;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupInputComponent() override;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();

	UPROPERTY(EditDefaultsOnly, Category="Xist")
	TArray<float> SimSpeedOptions;

	UPROPERTY(VisibleInstanceOnly, Category="Xist")
	int32 SimSpeedIndex {INDEX_NONE};

private:
	FVector CachedDestination;

	bool bIsTouch; // Is it a touch device
	float FollowTime; // For how long it has been pressed
};


