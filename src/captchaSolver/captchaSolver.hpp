#pragma once

#include <chrono>
#include <future>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <httpClient/httpClient.hpp>

class CaptchaSolver
{
public:
	explicit CaptchaSolver(const std::string& apiKey);

	std::future<std::optional<std::string>> SolveHCaptcha(const std::string& siteKey,
											    const std::string& pageURL);

private:
	std::string apiKey_;
	HttpClient httpClient_;

	std::optional<std::string> createTask_(const std::string& siteKey, const std::string& pageUrl);
	std::optional<std::string> pollResult_(const std::string& taskId);
};
