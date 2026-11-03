#include "stdafx.h"
#include <stdio.h>
#include <wininet.h>
#include <commctrl.h>
#include "LauncherConfig.h"
#include "LauncherWindow.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")

namespace
{
	enum { ID_USER = 101, ID_PASS, ID_PLACE, ID_VERSION, ID_PLAY, ID_STATUS };

	struct State
	{
		std::string baseHost;
		LauncherResult* result;
		bool ok = false;
		HWND user = 0, pass = 0, place = 0, version = 0, play = 0, status = 0;
	};

	std::string ToUtf8(const std::wstring& w)
	{
		if (w.empty()) return std::string();
		int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), 0, 0, 0, 0);
		std::string s(n, '\0');
		WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, 0, 0);
		return s;
	}
	std::wstring FromUtf8(const std::string& s)
	{
		if (s.empty()) return std::wstring();
		int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), 0, 0);
		std::wstring w(n, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
		return w;
	}
	std::wstring GetText(HWND h)
	{
		int n = GetWindowTextLengthW(h);
		std::wstring s(n + 1, L'\0');
		GetWindowTextW(h, &s[0], n + 1);
		s.resize(n);
		return s;
	}
	std::string JsonEscape(const std::string& in)
	{
		std::string o;
		for (unsigned char c : in)
		{
			switch (c)
			{
			case '"': o += "\\\""; break;
			case '\\': o += "\\\\"; break;
			case '\n': o += "\\n"; break;
			case '\r': o += "\\r"; break;
			case '\t': o += "\\t"; break;
			default:
				if (c < 0x20) { char b[8]; sprintf_s(b, "\\u%04x", c); o += b; }
				else o += (char)c;
			}
		}
		return o;
	}
	std::string Trim(const std::string& s)
	{
		size_t a = s.find_first_not_of(" \t\r\n\""), b = s.find_last_not_of(" \t\r\n\"");
		return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
	}
	// Extract a string value for "field" from a flat JSON object; bare text is returned as-is.
	std::string ExtractTicket(const std::string& body)
	{
		std::string t = Trim(body);
		if (t.empty() || t[0] != '{') return t;
		std::string key = std::string("\"") + LAUNCHER_TICKET_FIELD + "\"";
		size_t p = body.find(key);
		if (p == std::string::npos) return std::string();
		p = body.find(':', p + key.size());
		if (p == std::string::npos) return std::string();
		p = body.find('"', p);
		if (p == std::string::npos) return std::string();
		std::string v;
		for (size_t i = p + 1; i < body.size(); ++i)
		{
			if (body[i] == '\\' && i + 1 < body.size()) { v += body[++i]; continue; }
			if (body[i] == '"') return v;
			v += body[i];
		}
		return std::string();
	}

	// returns HTTP status, or 0 on network failure
	DWORD HttpPostJson(const std::string& host, const wchar_t* path, const std::string& json, std::string& response)
	{
		DWORD status = 0;
		HINTERNET s = InternetOpenW(L"NewServerLauncher", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);
		if (!s) return 0;
		DWORD timeout = 15000;
		InternetSetOptionW(s, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionW(s, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
		InternetSetOptionW(s, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
		std::wstring whost = FromUtf8(host);
		HINTERNET c = InternetConnectW(s, whost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
		if (c)
		{
			HINTERNET r = HttpOpenRequestW(c, L"POST", path, 0, 0, 0,
				INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI, 0);
			if (r)
			{
				static const wchar_t hdr[] = L"Content-Type: application/json\r\nAccept: application/json, text/plain\r\n";
				if (HttpSendRequestW(r, hdr, (DWORD)-1, (LPVOID)json.data(), (DWORD)json.size()))
				{
					DWORD len = sizeof(status);
					HttpQueryInfoW(r, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &len, 0);
					char buf[2048]; DWORD got = 0;
					while (InternetReadFile(r, buf, sizeof(buf), &got) && got) { response.append(buf, got); if (response.size() > 65536) break; }
				}
				InternetCloseHandle(r);
			}
			InternetCloseHandle(c);
		}
		InternetCloseHandle(s);
		return status;
	}

	void SetStatus(State* st, const wchar_t* text) { SetWindowTextW(st->status, text); UpdateWindow(st->status); }

	void DoPlay(HWND hwnd, State* st)
	{
		std::wstring user = GetText(st->user), pass = GetText(st->pass), place = GetText(st->place);
		if (user.empty() || pass.empty()) { SetStatus(st, L"Enter your username and password."); return; }
		if (place.empty() || place.find_first_not_of(L"0123456789") != std::wstring::npos)
		{ SetStatus(st, L"Place ID must be a number."); return; }

		EnableWindow(st->play, FALSE);
		SetStatus(st, L"Logging in...");
		HCURSOR old = SetCursor(LoadCursor(0, IDC_WAIT));

		std::string json = "{\"username\":\"" + JsonEscape(ToUtf8(user)) + "\",\"password\":\"" + JsonEscape(ToUtf8(pass)) + "\"}";
		std::string body;
		DWORD code = HttpPostJson(st->baseHost, LAUNCHER_LOGIN_PATH, json, body);
		SecureZeroMemory(&json[0], json.size());
		SetCursor(old);

		if (code == 0) { SetStatus(st, L"Cannot reach the server."); EnableWindow(st->play, TRUE); return; }
		std::string ticket = (code == 200) ? ExtractTicket(body) : std::string();
		if (ticket.empty()) { SetStatus(st, L"Login failed. Check your username and password."); EnableWindow(st->play, TRUE); return; }

		// remember username only (never the password)
		HKEY k;
		if (RegCreateKeyExW(HKEY_CURRENT_USER, LAUNCHER_REG_KEY, 0, 0, 0, KEY_WRITE, 0, &k, 0) == ERROR_SUCCESS)
		{
			RegSetValueExW(k, L"LastUser", 0, REG_SZ, (const BYTE*)user.c_str(), (DWORD)((user.size() + 1) * sizeof(wchar_t)));
			RegCloseKey(k);
		}

		int sel = (int)SendMessageW(st->version, CB_GETCURSEL, 0, 0);
		if (sel < 0) sel = 0;
		char url[512];
		sprintf_s(url, LAUNCHER_PLACE_URL_FORMAT, st->baseHost.c_str(), ToUtf8(place).c_str());
		st->result->authUrl = "https://" + st->baseHost + LAUNCHER_AUTH_PATH;
		st->result->authTicket = ticket;
		st->result->script = url;
		st->result->clientVersion = ToUtf8(LAUNCHER_CLIENT_VERSIONS[sel]);
		st->ok = true;
		DestroyWindow(hwnd);
	}

	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		State* st = (State*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		switch (msg)
		{
		case WM_CREATE:
		{
			st = (State*)((CREATESTRUCTW*)lp)->lpCreateParams;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)st);
			HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			HINSTANCE hi = GetModuleHandleW(0);
			auto mk = [&](const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id, DWORD ex = 0) {
				HWND c = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, x, y, w, h, hwnd, (HMENU)(INT_PTR)id, hi, 0);
				SendMessageW(c, WM_SETFONT, (WPARAM)f, TRUE);
				return c;
			};
			mk(L"STATIC", L"Username", 0, 16, 16, 90, 18, -1);
			st->user = mk(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, 110, 14, 200, 22, ID_USER, WS_EX_CLIENTEDGE);
			mk(L"STATIC", L"Password", 0, 16, 46, 90, 18, -1);
			st->pass = mk(L"EDIT", L"", ES_AUTOHSCROLL | ES_PASSWORD | WS_TABSTOP, 110, 44, 200, 22, ID_PASS, WS_EX_CLIENTEDGE);
			mk(L"STATIC", L"Place ID", 0, 16, 76, 90, 18, -1);
			st->place = mk(L"EDIT", L"1", ES_AUTOHSCROLL | ES_NUMBER | WS_TABSTOP, 110, 74, 200, 22, ID_PLACE, WS_EX_CLIENTEDGE);
			mk(L"STATIC", L"Client", 0, 16, 106, 90, 18, -1);
			st->version = mk(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 110, 104, 200, 120, ID_VERSION);
			for (auto v : LAUNCHER_CLIENT_VERSIONS) SendMessageW(st->version, CB_ADDSTRING, 0, (LPARAM)v);
			SendMessageW(st->version, CB_SETCURSEL, (WPARAM)(sizeof(LAUNCHER_CLIENT_VERSIONS) / sizeof(*LAUNCHER_CLIENT_VERSIONS) - 1), 0);
			st->status = mk(L"STATIC", L"", 0, 16, 138, 294, 18, ID_STATUS);
			st->play = mk(L"BUTTON", L"Play", BS_DEFPUSHBUTTON | WS_TABSTOP, 110, 162, 96, 28, ID_PLAY);
			mk(L"BUTTON", L"Cancel", WS_TABSTOP, 214, 162, 96, 28, IDCANCEL);

			// prefill last username
			HKEY k; wchar_t buf[256] = {}; DWORD sz = sizeof(buf) - sizeof(wchar_t), type = 0;
			if (RegOpenKeyExW(HKEY_CURRENT_USER, LAUNCHER_REG_KEY, 0, KEY_READ, &k) == ERROR_SUCCESS)
			{
				if (RegQueryValueExW(k, L"LastUser", 0, &type, (LPBYTE)buf, &sz) == ERROR_SUCCESS && type == REG_SZ)
					SetWindowTextW(st->user, buf);
				RegCloseKey(k);
			}
			SetFocus(buf[0] ? st->pass : st->user);
			return 0;
		}
		case WM_COMMAND:
			if (LOWORD(wp) == ID_PLAY && st) { DoPlay(hwnd, st); return 0; }
			if (LOWORD(wp) == IDCANCEL) { DestroyWindow(hwnd); return 0; }
			break;
		case WM_CLOSE: DestroyWindow(hwnd); return 0;
		case WM_DESTROY: PostQuitMessage(0); return 0;
		}
		return DefWindowProcW(hwnd, msg, wp, lp);
	}
}

bool ShowLauncherWindow(HINSTANCE hInstance, const std::string& baseHost, LauncherResult& result)
{
	State st; st.baseHost = baseHost; st.result = &result;

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = L"NewServerLauncherWnd";
	RegisterClassW(&wc);

	RECT r = { 0, 0, 326, 206 };
	DWORD style = WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&r, style, FALSE);
	int w = r.right - r.left, h = r.bottom - r.top;
	int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2, y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

	HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW, wc.lpszClassName, LAUNCHER_WINDOW_TITLE, style, x, y, w, h, 0, 0, hInstance, &st);
	if (!hwnd) return false;
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);

	MSG m;
	while (GetMessageW(&m, 0, 0, 0) > 0)
	{
		if (!IsDialogMessageW(hwnd, &m)) { TranslateMessage(&m); DispatchMessageW(&m); }
	}
	UnregisterClassW(wc.lpszClassName, hInstance);
	return st.ok;
}
