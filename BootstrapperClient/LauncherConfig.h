#pragma once
// ---------------------------------------------------------------------------
// In-app launcher configuration. Edit to match your backend.
// Defaults are written for Supabase Auth (free tier) - see BACKEND.md.
// ---------------------------------------------------------------------------
 
// Full URLs (POST, JSON). Login must return 200 with a JSON field LAUNCHER_TICKET_FIELD.
#define LAUNCHER_LOGIN_URL      L"https://nslmbxczihbbrjgfbmva.supabase.co/auth/v1/token?grant_type=password"
#define LAUNCHER_SIGNUP_URL     L"https://nslmbxczihbbrjgfbmva.supabase.co/auth/v1/signup"   // set to L"" to hide Register
 
// Extra HTTP headers sent with both requests (each line ends in \r\n).
// Supabase needs its public "anon" key here. The anon key is meant to be public.
#define LAUNCHER_EXTRA_HEADERS  L"apikey: sb_publishable_MoiD2w9bRR3AE0bKLs-pcg_Q-iXjOO3\r\n"
 
// JSON field names used in the request body and in the response.
#define LAUNCHER_USER_FIELD     "email"
#define LAUNCHER_PASS_FIELD     "password"
#define LAUNCHER_TICKET_FIELD   "access_token"
#define LAUNCHER_USER_LABEL     L"Email"
 
// Passed to the game client. First %s = BaseHost (see Bootstrapper.rc IDS_BASEHOST).
#define LAUNCHER_AUTH_URL_FORMAT    "https://%s/Login/Negotiate.ashx"                                  // client -a
#define LAUNCHER_PLACE_URL_FORMAT   "https://%s/game/PlaceLauncher.ashx?request=RequestGame&placeId=%s" // client -j (2nd %s = place id)
 
// Client versions in the drop-down (folder names under the install dir).
static const wchar_t* const LAUNCHER_CLIENT_VERSIONS[] = { L"2017L", L"2018L", L"2020L", L"2021M" };
 
#define LAUNCHER_WINDOW_TITLE   L"NewServer Launcher"
#define LAUNCHER_REG_KEY        L"Software\\NewServerReg\\Launcher"
