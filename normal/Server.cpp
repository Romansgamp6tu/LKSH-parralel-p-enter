#include "Server.hpp"

Data global_data;

void Data::sync()
{
	teams_raw = get_teams(*https);
	std::set<int> players_ids;
	for (auto& team : teams_raw)
	{
		for (auto& player : team.players_ids)
		{
			players_ids.insert(player);
		}
	}

	matches = get_matches(*https);

	teams.clear();

	for (auto& team : teams_raw)
	{
		teams[team.name] = team;
	}

	premium_matches = get_premium_matches(*https);
}

Session::Session(tcp::socket skt) : socket(std::move(skt)) {}
void Session::begin()
{
	auto self = shared_from_this();
	http::async_read(socket, buffer, req, [self](beast::error_code ec, std::size_t) {if (!ec) self->handle();});
}
std::string Session::get_param(const std::string& query, const std::string& name) {
	std::regex re(name + "=([^&]*)");
	std::smatch match;
	if (std::regex_search(query, match, re) && match.size() > 1) return match[1];
	return "";
}
void Session::handle()
{
	std::string target = req.target();
	auto pos = target.find('?');
	std::string path = (pos == std::string::npos) ? target : target.substr(0, pos);
	std::string query = (pos == std::string::npos) ? "" : target.substr(pos + 1);

	res.version(req.version());
	res.keep_alive(false);
	if (path == "/versus")
	{
		try
		{
			int player1 = std::stoi(get_param(query, "player1_id"));
			int player2 = std::stoi(get_param(query, "player2_id"));
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			res.body() = std::string(reinterpret_cast<const char*>(u8R"({"count_of_meetings" : )")) 
				+ std::to_string(get_players_meetings(player1, player2, global_data.teams_raw, global_data.matches)) + reinterpret_cast<const char*>(u8R"(})");
		}
		catch (...)
		{
			res.result(http::status::not_found);
			res.set(http::field::content_type, "text/plain");
			res.body() = reinterpret_cast<const char*>(u8R"(Not found)");
		}
	}
	else if (path == "/stats")
	{
		try
		{
			std::string team_name = get_param(query, "team_name");
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			res.body() = std::string(reinterpret_cast<const char*>(u8R"({"win_count" : )")) + std::to_string(get_stat_results(team_name, global_data.teams, global_data.matches).win_count)
				+ reinterpret_cast<const char*>(u8R"(, "lose_count" : )") + std::to_string(get_stat_results(team_name, global_data.teams, global_data.matches).lose_count)
				+ reinterpret_cast<const char*>(u8R"(, "delta_goals" : )") + std::to_string(get_stat_results(team_name, global_data.teams, global_data.matches).delta_goals)
				+ reinterpret_cast<const char*>(u8R"(})");
		}
		catch (...)
		{
			res.result(http::status::not_found);
			res.set(http::field::content_type, "text/plain");
			res.body() = reinterpret_cast<const char*>(u8R"(Not found)");
		}
	}
	else if (path == "/goals")
	{
		try
		{
			int player_id = std::stoi(get_param(query, "player_id"));
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			std::string body;
			auto goals = get_goals_of_player(player_id, global_data.premium_matches);
			body.push_back(u8'[');
			for (int idx = 0; idx < goals.size(); idx++)
			{
				body += reinterpret_cast<const char*>(u8R"({"match":)") + std::to_string(goals[idx].match_id) + reinterpret_cast<const char*>(u8R"(,"time":)") 
					+ std::to_string(goals[idx].time) + reinterpret_cast<const char*>(u8R"(})");
				if (idx < goals.size() - 1) body.push_back(u8',');
			}
			body.push_back(u8']');
			
			res.body() = body;
		}
		catch (...)
		{
			res.result(http::status::not_found);
			res.set(http::field::content_type, "text/plain");
			res.body() = reinterpret_cast<const char*>(u8R"(Not found)");
		}
	}
	else
	{
		res.result(http::status::not_found);
		res.set(http::field::content_type, "text/plain");
		res.body() = reinterpret_cast<const char*>(u8R"(Not found)");
	}

	res.prepare_payload();

	auto self = shared_from_this();
	http::async_write(socket, res, [self](beast::error_code ec, std::size_t) { beast::error_code ignore_ec; self->socket.shutdown(tcp::socket::shutdown_send, ignore_ec); self->socket.close(ignore_ec);});
}

Server::Server(unsigned int port, net::io_context& ioc) : ioc(ioc), acceptor(ioc)
{
	tcp::endpoint ep(tcp::v4(), port);
	acceptor.open(ep.protocol());
	acceptor.set_option(net::socket_base::reuse_address(true));
	acceptor.bind(ep);
	acceptor.listen();
}
void Server::run()
{
	acceptor.async_accept([this](beast::error_code ec, tcp::socket socket) {if (!ec) std::make_shared<Session>(std::move(socket))->begin(); else std::cerr << "чёт жопа случилась: " << ec.message() << std::endl; this->run(); });
}