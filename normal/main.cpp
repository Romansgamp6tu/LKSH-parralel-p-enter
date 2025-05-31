#include "pch.h"
#include "HTTPSRequest.hpp"
#include "LKSHAPI.hpp"
#include "Command.hpp"

void print_json(const nlohmann::json& j, int indent = 2) {
    std::cout << j.dump(indent) << std::endl;
}

void server()
{
    
}

int main() {
#ifdef BOOST_WINDOWS
#include<Windows.h>
SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);
#endif
    /*
    std::cout << "😊😊😊😊😊";
    system("pause");
    */

    const std::string plain_key("a53a604c589851232232738b0596f1fc2add969dfff203ebc19259a46eabf24b");

    try
    {
        HTTPSRequest https("https://lksh-enter.ru");
        https.set_plain_token(plain_key);
        auto teams_raw = get_teams(https);
        std::set<int> players_ids;
        for (auto& team : teams_raw)
        {
            for (auto& player : team.players_ids)
            {
                players_ids.insert(player);
            }
        }
        std::vector<std::string> players_names;
        for (auto& player_id : players_ids)
        {
            auto player = get_player(https, player_id);
            players_names.push_back(player.name + ' ' + player.surname);
        }
        std::sort(players_names.begin(), players_names.end());
        for (auto& name : players_names)
        {
            std::cout << name << std::endl;
        }

        auto matches = get_matches(https);

        std::map<std::string, Team> teams;

        for (auto& team : teams_raw)
        {
            teams[team.name] = team;
        }

        process_cmd(std::cin, std::cout, teams, teams_raw, matches);
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    system("pause");
    return 0;
    
}