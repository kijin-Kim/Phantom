#pragma once

UENUM(BlueprintType)
enum class ECharacterActionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Dodge UMETA(DisplayName = "Dodge"),
	Attack UMETA(DisplayName = "Attack"),
	Aim UMETA(DisplayName = "Aim"),
	Climb UMETA(DisplayName = "Climb"),
	Parry UMETA(DisplayName = "Parry"),
	Execute UMETA(DisplayName = "Execute"),
	HitReact UMETA(DisplayName = "HitReact"),
	Dead UMETA(DisplayName = "Dead"),
	Max UMETA(hidden)
};

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	Stop UMETA(DisplayName = "Stop"),
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Max UMETA(hidden)
};


#define PHANTOM_PLAYER_NAME_TAG TEXT("Player")
#define PHANTOM_GENERIC_TEAM_ID_PLAYER 0
#define PHANTOM_GENERIC_TEAM_ID_ENEMY 1