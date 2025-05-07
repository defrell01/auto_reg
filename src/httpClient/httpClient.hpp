#pragma once
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <future>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <proxyParser/proxyParser.hpp>

class HttpClient
{
public:
	explicit HttpClient(std::vector<Proxy> proxies = {});

	std::future<std::optional<nlohmann::json>> GetJson(const std::string& url);
	std::future<std::optional<nlohmann::json>> PostJson(const std::string& url,
											  const nlohmann::json& jsonData);

private:
	std::vector<Proxy> proxies_;

	Proxy pickRandomProxy_();
	std::future<std::optional<std::string>> sendRequest_(const std::string& method,
											   const std::string& url,
											   const std::optional<std::string>& data,
											   const std::vector<std::string>& headers);
};
