#include <captchaSolver/captchaSolver.hpp>

CaptchaSolver::CaptchaSolver(const std::string& apiKey)
	: apiKey_(std::move(apiKey)){};

std::future<std::optional<std::string>> CaptchaSolver::SolveHCaptcha(const std::string& siteKey,
													    const std::string& pageURL)
{
	return std::async(std::launch::async, [=]() -> std::optional<std::string> {
		auto taskIdOpt = createTask_(siteKey, pageURL);
		if(!taskIdOpt)
		{
			std::cerr << "Failed to create captcha task\n";
			return std::nullopt;
		}

		return pollResult_(*taskIdOpt);
	});
}

std::optional<std::string> CaptchaSolver::createTask_(const std::string& siteKey,
										    const std::string& pageUrl)
{
	nlohmann::json obj;
	obj["type"] = "HCaptchaTaskProxyless";
	obj["websiteURL"] = pageUrl;
	obj["websiteKey"] = siteKey;

	nlohmann::json payload;
	payload["clientKey"] = apiKey_;
	payload["task"] = obj;

	auto responseOpt =
		httpClient_.PostJson("https://api.solvecaptcha.com/createTask", payload).get();
	if(!responseOpt)
		return std::nullopt;

	try
	{
		auto response = *responseOpt;
		if(response.contains("taskId"))
		{
			return response["taskId"].get<std::string>();
		}
		std::cerr << "CreateTask error: " << *responseOpt << "\n";
	}
	catch(...)
	{
		std::cerr << "Failed to parse CreateTask response\n";
	}

	return std::nullopt;
}

std::optional<std::string> CaptchaSolver::pollResult_(const std::string& taskId)
{
	const std::string url = "https://api.solvecaptcha.com/getTaskResult";
	nlohmann::json payload;
	payload["clientKey"] = apiKey_;
	payload["taskId"] = taskId;

	for(int attempt = 0; attempt < 24; ++attempt)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));

		auto responseOpt = httpClient_.PostJson(url, payload).get();
		if(!responseOpt)
			continue;

		try
		{
			auto response = *responseOpt;
			if(response["status"] == "ready")
			{
				return response["solution"]["gRecaptchaResponse"].get<std::string>();
			}
			else if(response["status"] == "failed")
			{
				std::cerr << "Captcha solving failed: " << *responseOpt << "\n";
				return std::nullopt;
			}
		}
		catch(...)
		{
			std::cerr << "Failed to parse GetTaskResult response\n";
		}
	}

	std::cerr << "Captcha solving timed out\n";
	return std::nullopt;
}
