#include <proxyParser/proxyParser.hpp>
#include <httpClient/httpClient.hpp>

int main()
{
     ProxyParser parser{};
     auto proxies = parser.ParseFromFile("./proxy.txt");

	HttpClient client(proxies);

	auto response = client.Get("https://httpbin.org/ip").get();

	if(response)
	{
		std::cout << "Response: " << *response << "\n";
	}
	else
	{
		std::cerr << "Err\n";
	}
}
