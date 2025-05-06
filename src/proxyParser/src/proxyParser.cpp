#include <proxyParser/proxyParser.hpp>

std::string Proxy::ToCurlProxy() const
{
	return address;
}

std::optional<std::string> Proxy::ToCulrUserPwd() const
{
	if(username && password)
	{
		return username.value() + ":" + password.value();
	}
	return std::nullopt;
}

long Proxy::CurlProxyType() const
{
	switch(type)
	{
	case ProxyType::HTTP:
		return CURLPROXY_HTTP;
	case ProxyType::SOCKS5:
		return CURLPROXY_SOCKS5;
	default:
		return CURLPROXY_HTTP;
	}
}

std::vector<Proxy> ProxyParser::ParseFromFile(const std::string& path)
{
	std::vector<Proxy> result;
	std::ifstream file(path);
	if(!file.is_open())
	{
		std::cerr << "Cannot open proxy file: " << path << "\n";
		return {};
	}
	std::string line;

	while(std::getline(file, line))
	{
		if(line.empty())
			break;
		;

		std::string original = line;
		std::string typePrefix;
		ProxyType proxyType = ProxyType::HTTP;

		if(line.find("socks5://") == 0)
		{
			proxyType = ProxyType::SOCKS5;
			line = line.substr(9);
		}
		else if(line.find("http://") == 0)
		{
			proxyType = ProxyType::HTTP;
			line = line.substr(7);
		}

		std::replace(line.begin(), line.end(), ':', ' ');
		std::istringstream stream(line);

		std::string ip, port, user, pass;
		stream >> ip >> port;

		if(stream >> user >> pass)
		{
			result.push_back({ip + ":" + port, user, pass, proxyType});
		}
		else
		{
			result.push_back({ip + ":" + port, std::nullopt, std::nullopt, proxyType});
		}
	}

	return result;
}
