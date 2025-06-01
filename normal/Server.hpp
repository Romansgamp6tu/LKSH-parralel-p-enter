#pragma once
#include "pch.h"
#include "Versus.hpp"
#include "Stats.hpp"
#include "Goals.hpp"

struct Data
{
	std::vector<Team> teams_raw;
	std::vector<Match> matches;
	std::vector<MatchPremium> premium_matches;
	std::map<std::string, Team> teams;
	HTTPSRequest* https;
	void sync();
};

extern Data global_data;

class Session : public std::enable_shared_from_this<Session>
{
public:
	
	Session(tcp::socket skt);

	void begin();

private:

	std::string get_param(const std::string& query, const std::string& name);

	void handle();

	tcp::socket socket;
	beast::flat_buffer buffer;
	http::request<http::string_body> req;
	http::response<http::string_body> res;

};

class Server
{
public:
	Server(unsigned int port, net::io_context& ioc);
	void run();
private:
	net::io_context& ioc;
	tcp::acceptor acceptor;
};