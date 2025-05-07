#include <httpClient/httpClient.hpp>
#include <proxyParser/proxyParser.hpp>

int main()
{
	ProxyParser parser{};
	auto proxies = parser.ParseFromFile("./proxy.txt");

	HttpClient client(proxies);

	auto response = client.GetJson("https://httpbin.org/ip").get();

	if(response)
	{
		std::cout << "Response: " << (*response).dump(-1) << "\n";
	}
	else
	{
		std::cerr << "Err\n";
	}
}
