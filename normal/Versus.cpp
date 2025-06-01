#include "Versus.hpp"

int get_players_meetings(int id1, int id2, std::vector<Team> teams_raw, std::vector<Match> matches)
{
    std::vector<int> tp1, tp2;
    for (auto& team : teams_raw)
    {
        auto ip1 = std::find(team.players_ids.begin(), team.players_ids.end(), id1);
        auto ip2 = std::find(team.players_ids.begin(), team.players_ids.end(), id2);
        if (ip1 != team.players_ids.end() and ip2 == team.players_ids.end())
        {
            tp1.push_back(team.id);
        }
        if (ip2 != team.players_ids.end() and ip1 == team.players_ids.end())
        {
            tp2.push_back(team.id);
        }
    }
    int res = 0;
    for (auto& team1 : tp1)
    {
        for (auto& team2 : tp2)
        {
            for (auto& match : matches)
            {
                if (match.team1_id == team1 and match.team2_id == team2) res++;
            }
        }
    }
    return res;
}