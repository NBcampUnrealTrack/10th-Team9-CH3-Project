#include "Zombiemaid69GameState.h"

bool AZombiemaid69GameState::IsStageCleared() const
{
	return bStageCleared;
}

void AZombiemaid69GameState::MarkStageCleared()
{
	bStageCleared = true;
}
