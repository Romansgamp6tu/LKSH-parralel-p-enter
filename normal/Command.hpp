#pragma once
#include"pch.h"
#include"Versus.hpp"
#include"Stats.hpp"

void process_cmd(std::istream& in, std::ostream& out, std::map<std::string, Team>& teams, std::vector<Team>& teams_raw, std::vector<Match>& matches);