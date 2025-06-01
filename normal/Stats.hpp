#pragma once
#include "pch.h"
#include "LKSHAPI.hpp"

struct StatResults
{
    int win_count;
    int lose_count;
    int delta_goals;
};

StatResults get_stat_results(std::string team, std::map<std::string, Team> teams, std::vector<Match> matches);