#pragma once

#define OUTLINE_PURPLE 250
#define OUTLINE_BLUE 251
#define OUTLINE_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssualtRifle UMETA(DisplayName = "Assault Rifle"),

	EWT_MAX UMETA(DisplayName = "Default Max")
};
