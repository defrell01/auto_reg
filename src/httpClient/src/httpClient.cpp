#include <httpClient/httpClient.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	reinterpret_cast<std::string*>(userp)->append(reinterpret_cast<char*>(contents), size * nmemb);
	return size * nmemb;
}

HttpClient::HttpClient(std::vector<Proxy> proxies)
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

std::future<std::optional<json>> HttpClient::GetJson(const std::string& url)
{
	return std::async(std::launch::async, [this, url]() -> std::optional<json> {
		auto response = sendRequest_("GET", url, std::nullopt, {});
		auto str = response.get();
		if(!str)
			return std::nullopt;

		try
		{
			return json::parse(*str);
		}
		catch(...)
		{
			std::cerr << "Failed to parse GET JSON\n";
			return std::nullopt;
		}
	});
}

std::future<std::optional<json>> HttpClient::PostJson(const std::string& url, const json& jsonData)
{
	return std::async(std::launch::async, [this, url, jsonData]() -> std::optional<json> {
		std::string payload = jsonData.dump();
		auto headers = std::vector<std::string>{"Content-Type: application/json"};

		auto response = sendRequest_("POST", url, payload, headers);
		auto str = response.get();
		if(!str)
			return std::nullopt;

		try
		{
			return json::parse(*str);
		}
		catch(...)
		{
			std::cerr << "Failed to parse POST JSON\n";
			return std::nullopt;
		}
	});
}

std::future<std::optional<std::string>>
HttpClient::sendRequest_(const std::string& method,
					const std::string& url,
					const std::optional<std::string>& data,
					const std::vector<std::string>& headers)
{
	return std::async(std::launch::async, [=]() -> std::optional<std::string> {
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

		if(method == "POST")
		{
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			if(data.has_value())
			{
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->c_str());
			}
		}
		else if(method != "GET")
		{
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
			if(data.has_value())
			{
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->c_str());
			}
		}

		// Headers
		struct curl_slist* chunk = nullptr;
		for(const auto& h : headers)
		{
			chunk = curl_slist_append(chunk, h.c_str());
		}
		if(chunk)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}

		// Misc
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		CURLcode res = curl_easy_perform(curl);
		if(chunk)
			curl_slist_free_all(chunk);
		curl_easy_cleanup(curl);

		if(res != CURLE_OK)
		{
			std::cerr << "CURL " << method << " error: " << curl_easy_strerror(res) << "\n";
			return std::nullopt;
		}

		return response;
	});
}