# Rebranded bootstrapper

Generated from Korone-Bootstrapper 1.7.0.4 with `rebrand.py` (included one level up).
Re-run it with your real values:

    python3 rebrand.py <original-src> <out-dir> --name "YourName" --domain yourdomain.tld

What changes: display names, window titles, exe/DLL names, registry keys, mutex/event names,
protocol schemes (`<name>-player`, `<name>-studio`), install host (`setup.<domain>`),
base host (`www.<domain>`), API host (`api.<domain>`), allowed-domains entries, CI artifact names.

## Your server must provide (same layout the original expects)
- `setup.<domain>/` : version file(s), `<ver>-<Name>.exe`, `RobloxProxy.zip`, `NPRobloxProxy.zip`,
  `ProjectXApp2017L/2018L/2020L/2021M.zip`, plus `*-rbxManifest.txt`; ETag/MD5 headers on each.
- `www.<domain>/Login/Negotiate.ashx`, `/game/...` endpoints, `/download/thankyou`, `/games`.
- `api.<domain>` (services use `<service>.api.<domain>`-style hosts via RobloxServicesTools.cpp).

## Not changed (deliberately)
- COM CLSIDs / GUIDs: they must match the proxy DLLs you ship in RobloxProxy.zip. If you rebuild
  those, regenerate the GUIDs in BootstrapperClient.cpp and SharedHelpers.cpp together.
- Images in Bootstrapper/assets/*.bmp still carry the old logo: replace with your own art.
- Studio/legacy branches are kept as-is.

Build: Visual Studio 2022 + vcpkg, Release x86/x64, project `BootstrapperClient` (Windows only; not built here).

## Built-in launcher window (no website needed)
Run `NewServerPlayerLauncher.exe` by hand (shortcut / double-click): after the install/update check it
shows a window with Username, Password, Place ID and Client version, then starts the game itself.
Code: `BootstrapperClient/LauncherWindow.cpp/.h`, hook in `StartRobloxApp()` (BootstrapperClient.cpp).
Protocol-link launches from a website still work exactly as before.

Edit `BootstrapperClient/LauncherConfig.h` to match your server. Server contract:
- `POST https://www.<domain>/Login/Launcher.ashx` with JSON `{"username","password"}` -> 200 + `{"ticket":"..."}` (or bare ticket text)
- Client is started with `-a https://www.<domain>/Login/Negotiate.ashx -t <ticket> -j <PlaceLauncher URL for the place id>`
- Password is only sent over HTTPS to your host and is never stored; only the last username is remembered (HKCU).

NOTE: re-running rebrand.py on the ORIGINAL source does not include this feature. To change the
name/domain after this, edit this tree directly (or re-run rebrand.py on it with the old names).
