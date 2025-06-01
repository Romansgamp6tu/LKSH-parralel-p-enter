#include "LKSHAPI.hpp"
std::vector<Player> get_players(HTTPSRequest& req)
{
    std::vector<Player> players;
    try //да, € гений останавливать цикл исключением =))    (€ понимаю, что так лучше не делать, но и так сойдЄт)
        //крч, объ€сн€ю, € просто нахожу всех игроков до того момента, пока € не наткнулс€ на ошибку 404 - значит список закончисл€ =/ (не фиксите пж)
    {
        int i = 1;
        for (;;)
        {
            nlohmann::json player = req.Request(http::verb::get, "players/" + std::to_string(i));
            players.push_back({ int(player["id"]), std::string(player["name"]), int(player["number"]), std::string(player["surname"]) });
            i++;
        }
    }
    catch (std::system_error& err)
    {
        if (err.code().value() != 404) throw err;
    }
    return players;
}
Player get_player(HTTPSRequest& req, int player_id)
{
    nlohmann::json player = req.Request(http::verb::get, "players/" + std::to_string(player_id));
    return { int(player["id"]), std::string(player["name"]), int(player["number"]), std::string(player["surname"]) };
}

std::vector<Match> get_matches(HTTPSRequest& req)
{
    std::vector<nlohmann::json> matches_array = req.Request(http::verb::get, "matches").get<std::vector<nlohmann::json>>();
    std::vector<Match> matches;
    matches.reserve(matches_array.size() + 1);
    for (auto& match : matches_array)
    {
        matches.push_back({ int(match["id"]), int(match["team1"]), int(match["team1_score"]), int(match["team2"]), int(match["team2_score"]) });
    }
    return matches;
}

std::vector<MatchPremium> get_premium_matches(HTTPSRequest& req)
{
    std::vector<nlohmann::json> matches_array = req.Request(http::verb::get, "matches").get<std::vector<nlohmann::json>>();
    std::vector<MatchPremium> matches;
    matches.reserve(matches_array.size() + 1);
    for (auto& match : matches_array)
    {
        int match_id = int(match["id"]);
        std::vector<nlohmann::json> goals_array = req.Request(http::verb::get, "goals?match_id=" + std::to_string(match_id)).get<std::vector<nlohmann::json>>();
        std::vector<Goal> goals;
        goals.reserve(goals_array.size() + 1);
        for (auto& i : goals_array)
        {
            goals.push_back({ int(i["id"]), int(i["player"]), int(i["match"]), int(i["minute"]) });
        }
        matches.push_back({ match_id, int(match["team1"]), int(match["team1_score"]), int(match["team2"]), int(match["team2_score"]), goals });
    }
    return matches;
}

std::vector<Goal> get_goals_of_match(HTTPSRequest& req, int match_id)
{
    std::vector<nlohmann::json> goals_array = req.Request(http::verb::get, "goals?match_id=" + std::to_string(match_id)).get<std::vector<nlohmann::json>>();
    std::vector<Goal> goals;
    goals.reserve(goals_array.size() + 1);
    for (auto& i : goals_array)
    {
        goals.push_back({ int(i["id"]), int(i["player"]), int(i["match"]), int(i["minute"]) });
    }
    return goals;
}

std::vector<Team> get_teams(HTTPSRequest& req)
{
    std::vector<nlohmann::json> teams_array = req.Request(http::verb::get, "teams").get<std::vector<nlohmann::json>>();
    std::vector<Team> teams;
    teams.reserve(teams_array.size());
    for (auto& team : teams_array)
    {
        teams.push_back({ int(team["id"]), std::string(team["name"]), team["players"].get<std::vector<int>>() });
    }
    return teams;
}
Team get_team(HTTPSRequest& req, int team_id)
{
    nlohmann::json team = req.Request(http::verb::get, "teams/" + std::to_string(team_id));
    return { int(team["id"]), std::string(team["name"]), team["players"].get<std::vector<int>>() };
}

void authorise(HTTPSRequest& req, std::string reason)
{
    std::cout << std::string(reinterpret_cast<const char*>(u8R"({"reason":")")) + reason + std::string(reinterpret_cast<const char*>(u8R"("})")) << std::endl;
    req.post(std::string(reinterpret_cast<const char*>(u8R"({"reason":")")) + reason + std::string(reinterpret_cast<const char*>(u8R"("})")), "login");
}