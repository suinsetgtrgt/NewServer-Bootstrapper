#pragma once
#include <string>

struct LauncherResult
{
	std::string authUrl;       // full URL for client -a
	std::string authTicket;    // client -t
	std::string script;        // client -j
	std::string clientVersion; // folder name, e.g. "2018L"
};

// Shows the login / play window. Returns true if the user logged in and pressed Play
// (result filled in), false if cancelled/closed. baseHost like "www.newserver.example".
bool ShowLauncherWindow(HINSTANCE hInstance, const std::string& baseHost, LauncherResult& result);
