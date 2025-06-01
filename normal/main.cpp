#include "pch.h"
#include "HTTPSRequest.hpp"
#include "LKSHAPI.hpp"
#include "Command.hpp"
#include "Server.hpp"

void client()
{
    HTTPSRequest https(SERVER_NAME);
    https.set_plain_token(PLAIN_KEY);
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

void server()
{
    net::io_context ioc;
    global_data.https = new HTTPSRequest(SERVER_NAME);
    global_data.https->set_plain_token(PLAIN_KEY);
    global_data.sync();
    auto srv = std::make_shared<Server>(8080, ioc);
    srv->run();
    ioc.run();
}

int main() {
#ifdef BOOST_WINDOWS
#if BOOST_WINDOWS
#include<Windows.h>
SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);
#endif
#endif
    try
    {
#ifdef SERVER
#ifdef CLIENT
#error "could not compile server and client in 1 programm!"
#endif // CLIENT
        server();
#elifdef CLIENT
        client();
#else
#error "could not compile nothing!"
#endif

    }
    catch (std::exception& e)
    {
        std::cerr << "Programm downed with error: " << e.what() << std::endl;
    }
    system("pause");
    return 0;
    
}