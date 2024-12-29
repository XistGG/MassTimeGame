// Copyright (c) 2025 Xist.GG

#include "MTGGameMode.h"

#include "MTGCharacter.h"
#include "MTGPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AMTGGameMode::AMTGGameMode()
{
	DefaultPawnClass = AMTGCharacter::StaticClass();
	PlayerControllerClass = AMTGPlayerController::StaticClass();

	// set default pawn to our BP character
	static ConstructorHelpers::FClassFinder<APawn> CharacterBPClass(TEXT("/Game/Blueprints/BP_Character"));
	if(CharacterBPClass.Class != nullptr)
	{
		DefaultPawnClass = CharacterBPClass.Class;
	}

	// set default controller to our BP controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/BP_PlayerController"));
	if(PlayerControllerBPClass.Class != nullptr)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}
