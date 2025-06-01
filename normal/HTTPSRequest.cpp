#include "HTTPSRequest.hpp"


HTTPSRequest::parsed_url HTTPSRequest::parse_url(const std::string& url)
{
	std::regex url_regex(R"((https?)://([^/:]+)(?::(\d+))?(/.*)?)");
	std::smatch match;
	if (std::regex_match(url, match, url_regex))
	{
		parsed_url result;
		result.scheme = match[1];
		result.host = match[2];
		result.port = match[3].matched ? match[3].str() : (result.scheme == "https" ? "443" : "80");
		result.target = match[4].matched ? match[4].str() : "/";
		return result;
	}
	throw std::invalid_argument("Invalid URL: " + url);
}

HTTPSRequest::HTTPSRequest(std::string _url, unsigned int http_version) : url(parse_url(_url)), resolver(ioc), http_version(http_version)
{
	if (url.scheme == "https")
	{
		is_safe = true;
		safe_ctx = std::make_unique<ssl::context>(ssl::context::sslv23_client);
		auto tcp = beast::tcp_stream(ioc);
		auto results = resolver.resolve(url.host, url.port);
		tcp.connect(results);
		safe_stream = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(std::move(tcp), *safe_ctx);
		if (!SSL_set_tlsext_host_name(safe_stream->native_handle(), url.host.c_str()))
		{
			beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
			throw beast::system_error{ ec };
		}
		safe_stream->handshake(ssl::stream_base::client);
	}
	else if (url.scheme == "http")
	{
		is_safe = false;
		stream = std::make_unique<beast::tcp_stream>(ioc);
		auto results = resolver.resolve(url.host, url.port);
		stream->connect(results);
	}
	else
	{
		throw std::runtime_error("Invalid scheme for url!");
	}
}
HTTPSRequest::~HTTPSRequest()
{
	if (is_safe)
	{
		beast::error_code ec;
		safe_stream->shutdown(ec);
	}
	else
	{
		beast::error_code ec;
		stream->socket().shutdown(tcp::socket::shutdown_both, ec);
	}
}
nlohmann::json HTTPSRequest::Request(http::verb http_method, std::string target)
{
	http::request<http::string_body> req{ http_method, url.target + target, http_version };

	//std::cout << url.target + target << std::endl;
#ifndef SHUTUP429
	req.set(http::field::host, url.host);
	req.set(http::field::user_agent, have_agent_name ? agent_name : BOOST_BEAST_VERSION_STRING);
	req.set(http::field::accept, "application/json; charset=utf-8");

	if (have_plain_token) req.set(http::field::authorization, plain_token);

	nlohmann::json result;

	if (is_safe)
	{
		http::write(*safe_stream, req);

		beast::flat_buffer buffer;
		http::response<http::string_body> res;
		http::read(*safe_stream, buffer, res);
		if (res.result() == http::status::ok)
		{
			result = nlohmann::json::parse(res.body());
		}
		else
		{
			throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
			//std::cerr << "HTTP error: " << res.result_int() << " - " << res.reason() << std::endl;
			//std::cerr << "Body: " << res.body() << std::endl;
		}
	}
	else
	{
		http::write(*stream, req);

		beast::flat_buffer buffer;
		http::response<http::string_body> res;
		http::read(*stream, buffer, res);
		if (res.result() == http::status::ok)
		{
			result = nlohmann::json::parse(res.body());
		}
		else
		{
			throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
			//std::cerr << "HTTP error: " << res.result_int() << " - " << res.reason() << std::endl;
			//std::cerr << "Body: " << res.body() << std::endl;
		}
	}
#else
	nlohmann::json result;
	http::response<http::string_body> res;

	do
	{
		req.set(http::field::host, url.host);
		req.set(http::field::user_agent, have_agent_name ? agent_name : BOOST_BEAST_VERSION_STRING);
		req.set(http::field::accept, "application/json; charset=utf-8");

		if (have_plain_token) req.set(http::field::authorization, plain_token);

		if (is_safe)
		{
			http::write(*safe_stream, req);
			res.body().clear();
			beast::flat_buffer buffer;
			http::read(*safe_stream, buffer, res);
			if (res.result() == http::status::ok)
			{
				try
				{
					result = nlohmann::json::parse(res.body());
				}
				catch (std::exception& err)
				{
					std::cerr << "JSON parsing failure: " << err.what() << " JSON body: " << res.body() << std::endl;
				}
			}
			else if(res.result() != http::status::too_many_requests)
			{
				throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
				//std::cerr << "HTTP error: " << res.result_int() << " - " << res.reason() << std::endl;
				//std::cerr << "Body: " << res.body() << std::endl;
			}
		}
		else
		{
			http::write(*stream, req);

			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(*stream, buffer, res);
			if (res.result() == http::status::ok)
			{
				result = nlohmann::json::parse(res.body());
			}
			else if(res.result() != http::status::too_many_requests)
			{
				throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
				//std::cerr << "HTTP error: " << res.result_int() << " - " << res.reason() << std::endl;
				//std::cerr << "Body: " << res.body() << std::endl;
			}
		}
		if (res.result() == http::status::too_many_requests)
		{
			//std::clog << "429 SHUTR UP =))" << std::endl;
			time_t t = clock();
			while (clock() - t < 2'000) {}
		}
	} while (res.result() == http::status::too_many_requests);
#endif

	return result;
}
void HTTPSRequest::set_agent_name(std::string new_agent_name)
{
	agent_name = new_agent_name;
	have_agent_name = true;
}
void HTTPSRequest::delete_agent_name(std::string new_agent_name)
{
	have_agent_name = false;
}
void HTTPSRequest::set_plain_token(std::string new_plain_token)
{
	plain_token = new_plain_token;
	have_plain_token = true;
}
void HTTPSRequest::delete_plain_token(std::string new_plain_token)
{
	have_plain_token = false;
}