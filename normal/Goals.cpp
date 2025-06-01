#include "Goals.hpp"

std::vector<PlayerGoal> get_goals_of_player(int player_id, std::vector<MatchPremium> matches)
{
	std::vector<PlayerGoal> res;
	for (auto& match : matches)
	{
		for (auto& goal : match.goals)
		{
			if (goal.player_id == player_id)
			{
				res.push_back({ goal.match_id, goal.minute });
			}
		}
	}
	return res;
}