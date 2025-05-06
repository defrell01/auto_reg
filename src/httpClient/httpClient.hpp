#pragma once
#include <curl/curl.h>
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
	explicit HttpClient(const std::vector<Proxy>& proxies);

	std::future<std::optional<std::string>> Get(const std::string& url);
	std::future<std::optional<std::string>> Post(const std::string& url, const std::string& body);

private:
	Proxy pickRandomProxy_();

	std::vector<Proxy> proxies_;
	std::mt19937 rng_;
};
