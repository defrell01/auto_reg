#include <httpClient/httpClient.hpp>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	reinterpret_cast<std::string*>(userp)->append(reinterpret_cast<char*>(contents), size * nmemb);
	return size * nmemb;
}

HttpClient::HttpClient(const std::vector<Proxy>& proxies)
	: proxies_(std::move(proxies))
{
	curl_global_init(CURL_GLOBAL_ALL);
}

Proxy HttpClient::pickRandomProxy_()
{
	if(proxies_.empty())
		return {};
	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, proxies_.size() - 1);
	return proxies_[dist(rng)];
}

std::future<std::optional<std::string>> HttpClient::Get(const std::string& url)
{
	return std::async(std::launch::async, [this, url]() -> std::optional<std::string> {
		CURL* curl = curl_easy_init();
		if(!curl)
			return std::nullopt;

		std::string response;
		Proxy proxy = pickRandomProxy_();

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if(!proxy.address.empty())
		{
               std::cout << proxy.ToCurlProxy() << "\n";

			curl_easy_setopt(curl, CURLOPT_PROXY, proxy.ToCurlProxy().c_str());
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxy.CurlProxyType());

			if(auto userpwd = proxy.ToCulrUserPwd(); userpwd.has_value())
			{
				curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userpwd->c_str());
			}
		}

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(res != CURLE_OK)
		{
			std::cerr << "CURL GET error: " << curl_easy_strerror(res) << "\n";
			return std::nullopt;
		}

		return response;
	});
}

std::future<std::optional<std::string>> HttpClient::Post(const std::string& url,
											  const std::string& data)
{
	return std::async(std::launch::async, [this, url, data]() -> std::optional<std::string> {
		CURL* curl = curl_easy_init();
		if(!curl)
			return std::nullopt;

		std::string response;
		Proxy proxy = pickRandomProxy_();

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if(!proxy.address.empty())
		{
			curl_easy_setopt(curl, CURLOPT_PROXY, proxy.ToCurlProxy().c_str());
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxy.CurlProxyType());

			if(auto userpwd = proxy.ToCulrUserPwd(); userpwd.has_value())
			{
				curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, userpwd->c_str());
			}
		}

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		CURLcode res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if(res != CURLE_OK)
		{
			std::cerr << "CURL POST error: " << curl_easy_strerror(res) << "\n";
			return std::nullopt;
		}

		return response;
	});
}