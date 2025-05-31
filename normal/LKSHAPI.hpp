#pragma once
#include "HTTPSRequest.hpp"

struct Player
{
    int id;
    std::string name;
    int number;
    std::string surname;
};

std::vector<Player> get_players(HTTPSRequest& req);
Player get_player(HTTPSRequest& req, int player_id);


struct Match
{
    int id;
    int team1_id;
    int team1_score;
    int team2_id;
    int team2_score;
};

std::vector<Match> get_matches(HTTPSRequest& req);


struct Team
{
    int id;
    std::string name;
    std::vector<int> players_ids;
};

std::vector<Team> get_teams(HTTPSRequest& req);
Team get_team(HTTPSRequest& req, int team_id);