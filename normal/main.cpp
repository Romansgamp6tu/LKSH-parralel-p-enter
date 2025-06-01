#include "pch.h"
#include "HTTPSRequest.hpp"
#include "LKSHAPI.hpp"
#include "Command.hpp"
#include "Server.hpp"

void print_json(nlohmann::json j)
{
    for (int i = 0; i < j.size(); i++)
    {
        std::cout << j.dump();
    }
}

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
    std::clog << "Starting server..." << std::endl;
    net::io_context ioc;
    global_data.https = new HTTPSRequest(SERVER_NAME);
    global_data.https->set_plain_token(PLAIN_KEY);
    std::clog << "collecting data..." << std::endl;
    global_data.sync();
    std::clog << "Server started on port " << SERVER_PORT << std::endl;
    auto srv = std::make_shared<Server>(SERVER_PORT, ioc);
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
        //authorise(https,std::string(reinterpret_cast<const char*>(u8R"(Тем, кто знают как работает промка, этот курс не нужен, они же ведь знают. Я - тот кто хочет научиться этому исскуству.)")));
        //print_json(https.Request(http::verb::get, "goals?match_id=5"));
    }
    catch (std::exception& e)
    {
        std::cerr << "Programm downed with error: " << e.what() << std::endl;
    }
    system("pause");
    return 0;
    
}