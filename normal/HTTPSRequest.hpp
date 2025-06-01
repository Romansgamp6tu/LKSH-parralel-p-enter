#pragma once
#include "pch.h"
class HTTPSRequest
{
public:

    HTTPSRequest(std::string _url, unsigned int http_version = 11);

    HTTPSRequest(HTTPSRequest&) = delete;
    HTTPSRequest(HTTPSRequest&&) = delete;

    ~HTTPSRequest();

    nlohmann::json Request(http::verb http_method, std::string target = "");

    void set_agent_name(std::string new_agent_name);
    void delete_agent_name(std::string new_agent_name);

    void set_plain_token(std::string new_plain_token);
    void delete_plain_token(std::string new_plain_token);

    void post(std::string what, std::string target = "")
    {
        http::request<http::string_body> req{ http::verb::post, url.target + target, http_version };

        //std::cout << url.target + target << std::endl;

        req.set(http::field::host, url.host);
        req.set(http::field::user_agent, have_agent_name ? agent_name : BOOST_BEAST_VERSION_STRING);

        if (have_plain_token) req.set(http::field::authorization, plain_token);

        req.body() = what;
        req.set(http::field::content_type, "application/json");
        req.prepare_payload();

        if (is_safe)
        {
            http::write(*safe_stream, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(*safe_stream, buffer, res);
            if (res.result() != http::status::ok)
                throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
        }
        else
        {
            http::write(*stream, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(*stream, buffer, res);
            if (res.result() != http::status::ok)
                throw std::system_error(res.result_int(), std::system_category(), std::string("HTTP error: ") + std::to_string(res.result_int()) + " - " + res.reason().data() + "\nBody: " + res.body().data());
        }

    }

protected:

    struct parsed_url
    {
        std::string scheme;
        std::string host;
        std::string port;
        std::string target;
    };

    parsed_url parse_url(const std::string& url);//simplier work with url's

private:

    parsed_url url;//url

    net::io_context ioc;// boost io context

    tcp::resolver resolver;// tcp resolver

    std::unique_ptr<beast::tcp_stream> stream;// http stream

    std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> safe_stream; //https stream
    std::unique_ptr<ssl::context> safe_ctx;//https context
    std::optional<bool> is_safe;//is https

    const int http_version;//http/https version

    bool have_agent_name;
    std::string agent_name;

    bool have_plain_token;
    std::string plain_token;
};