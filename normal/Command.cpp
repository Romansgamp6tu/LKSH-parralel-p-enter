#include "Command.hpp"

void process_cmd(std::istream& in, std::ostream& out, std::map<std::string, Team>& teams, std::vector<Team>& teams_raw, std::vector<Match>& matches)
{
    while (!in.eof())
    {
        try
        {
            std::string command;
            std::getline(in, command);
            if (command.substr(0, 6) == "stats?")
            {
                command.erase(command.begin(), command.begin() + 8);
                command.pop_back();
                //готовое название тимы
                auto stats = get_stat_results(command, teams, matches);

                out << stats.win_count << ' ' << stats.lose_count << ' ' << stats.delta_goals << std::endl;
            }
            else if (command.substr(0, 7) == "versus?")
            {
                command.erase(command.begin(), command.begin() + 8);
                std::stringstream pas; // players against stream
                pas << command;
                int id1, id2;
                pas >> id1 >> id2;
                auto res = get_players_meetings(id1, id2, teams_raw, matches);
                out << res << std::endl;
            }
        }
        catch (std::exception& err)
        {
            out << "Error has ocuurted: " << err.what();
        }
    }
}