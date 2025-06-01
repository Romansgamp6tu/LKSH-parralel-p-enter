#include "pch.h"
#include "LKSHAPI.hpp"

struct PlayerGoal
{
	int match_id;
	int time;
};

std::vector<PlayerGoal> get_goals_of_player(int player_id, std::vector<MatchPremium> matches);