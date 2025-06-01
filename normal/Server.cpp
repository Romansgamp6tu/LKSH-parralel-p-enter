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
	else if (path == "/front/stats")
	{
		try
		{
			res.result(http::status::ok);
			res.set(http::field::content_type, "text/html");
			std::string body(reinterpret_cast<const char*>(u8R"(<!DOCTYPE html><html lang="ru"><head><meta charset="UTF-8"><title>Team Statistics</title></head><body><h2>Team Statistics</h2><form id="statsForm" action="/front/stats" method="get" onsubmit="event.preventDefault(); goToStats();"><label for="team_name">Team name:</label><input type="text" id="team_name" name="team_name" required><button type="submit">Find statistics</button></form><div id="statsResult"></div><script> function goToStats() { const name = document.getElementById('team_name').value.trim(); if (name) { window.location.href = `/front/stats?team_name=${encodeURIComponent(name)}`; } } function getQueryParam(name) { const params = new URLSearchParams(window.location.search); return params.get(name); } const teamName = getQueryParam('team_name'); if (teamName) { fetch(`/stats?team_name=${encodeURIComponent(teamName)}`) .then(resp => resp.json()) .then(data => { document.getElementById('statsResult').innerHTML = ` <h3>Статистика для "${teamName}"</h3><ul><li>Побед: <b>${data.win_count}</b></li><li>Поражений: <b>${data.lose_count}</b></li><li>Разница голов: <b>${data.delta_goals}</b></li></ul> `; }) .catch(() => { document.getElementById('statsResult').innerHTML = `<span style="color:red">Ошибка получения статистики</span>`; }); } </script></body></html>)"));
			res.body() = body;
		}
		catch (...)
		{
			res.result(http::status::not_found);
			res.set(http::field::content_type, "text/plain");
			res.body() = reinterpret_cast<const char*>(u8R"(Not found)");
		}
	}
	else if (path == "/front/versus")
	{
		try
		{
			res.result(http::status::ok);
			res.set(http::field::content_type, "text/html");
			std::string body(reinterpret_cast<const char*>(u8R"(<!DOCTYPE html><html lang="ru"><head><meta charset="UTF-8"><title>Versus Players</title></head><body><h2>Versus: Find Matches Between Players</h2><form id="versusForm" action="/front/versus" method="get" onsubmit="event.preventDefault(); goToVersus();"><label for="player1_id">Player 1 ID:</label><input type="text" id="player1_id" name="player1_id" required><br><label for="player2_id">Player 2 ID:</label><input type="text" id="player2_id" name="player2_id" required><br><button type="submit">Find matches</button></form><div id="versusResult"></div><script> function goToVersus() { const id1 = document.getElementById('player1_id').value.trim(); const id2 = document.getElementById('player2_id').value.trim(); if (id1 && id2) { window.location.href = `/front/versus?player1_id=${encodeURIComponent(id1)}&player2_id=${encodeURIComponent(id2)}`; } } function getQueryParam(name) { const params = new URLSearchParams(window.location.search); return params.get(name); } const p1 = getQueryParam('player1_id'); const p2 = getQueryParam('player2_id'); if (p1 && p2) { fetch(`/versus?player1_id=${encodeURIComponent(p1)}&player2_id=${encodeURIComponent(p2)}`) .then(resp => resp.json()) .then(data => { document.getElementById('versusResult').innerHTML = ` <h3>Игроки ${p1} vs ${p2}</h3><p>Количество встреч: <b>${data.matches_count}</b></p> `; }) .catch(() => { document.getElementById('versusResult').innerHTML = `<span style="color:red">Ошибка получения данных</span>`; }); } </script></body></html>)"));
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