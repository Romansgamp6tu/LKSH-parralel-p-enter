#include "Stats.hpp"

StatResults get_stat_results(std::string team, std::map<std::string, Team> teams, std::vector<Match> matches)
{
    int team_id = teams[team].id;
    //статистика
    int win_count = 0, lose_count = 0, your_goals = 0, others_goals = 0;

    for (auto& match : matches)
    {
        if (team_id == match.team1_id)
        {
            win_count += match.team1_score > match.team2_score;
            lose_count += match.team2_score > match.team1_score;
            your_goals += match.team1_score;
            others_goals += match.team2_score;
        }
        else if (team_id == match.team2_id)
        {
            win_count += match.team2_score > match.team1_score;
            lose_count += match.team1_score > match.team2_score;
            your_goals += match.team2_score;
            others_goals += match.team1_score;
        }
    }
    return { win_count, lose_count, your_goals - others_goals };
}