#pragma once
#include <algorithm>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class ProxyType
{
	HTTP,
	SOCKS5
};

struct Proxy
{
	std::string address; // IP:PORT
	std::optional<std::string> username;
	std::optional<std::string> password;
	ProxyType type;

	std::string ToCurlProxy() const;
	std::optional<std::string> ToCulrUserPwd() const;

	long CurlProxyType() const;
};

class ProxyParser
{
public:
	static std::vector<Proxy> ParseFromFile(const std::string& path);
};
