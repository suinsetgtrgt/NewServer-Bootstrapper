#pragma once
// ---------------------------------------------------------------------------
// In-app launcher configuration.  Edit these to match YOUR server's API.
// All paths are relative to https://<BaseHost> (e.g. https://www.newserver.example).
// ---------------------------------------------------------------------------

// POST, Content-Type: application/json, body: {"username":"..","password":".."}
// Must answer HTTP 200 with either
//   - JSON containing  "ticket":"<auth ticket>"   (field name set below), or
//   - the bare ticket as plain text.
// Any non-200 answer is shown to the user as "Login failed".
#define LAUNCHER_LOGIN_PATH         L"/Login/Launcher.ashx"
#define LAUNCHER_TICKET_FIELD       "ticket"

// Passed to the game client as -a (the client calls this to exchange the ticket).
// Same endpoint the original website flow used.
#define LAUNCHER_AUTH_PATH          "/Login/Negotiate.ashx"

// Passed to the game client as -j. First %s = host, second %s = place id.
#define LAUNCHER_PLACE_URL_FORMAT   "https://%s/game/PlaceLauncher.ashx?request=RequestGame&placeId=%s"

// Client versions offered in the drop-down (folder names under the install dir;
// must match the ProjectXApp<ver>.zip packages deployed in DeployComponents).
static const wchar_t* const LAUNCHER_CLIENT_VERSIONS[] = { L"2017L", L"2018L", L"2020L", L"2021M" };

// Window title / registry key for remembering the last username.
#define LAUNCHER_WINDOW_TITLE       L"NewServer Launcher"
#define LAUNCHER_REG_KEY            L"Software\\NewServerReg\\Launcher"
