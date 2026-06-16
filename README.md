# XenonHTTPnoS

A Dashlaunch plugin to redirect and downgrade HTTP(S) requests on the Xbox 360. It hooks the `NetDll_XHttpConnect` function in `xam.xex`.

Used for [Arkham: Revived (Batman: Arkham Origins online)](https://github.com/KiwifruitDev/ArkhamRevivedWorker).

## Usage

`XenonHTTPnoS.ini`:

```ini
[Options]
DebuggerLogging = true

;section name can be anything
[ArkhamRevived]
; urls are not directly replaced, only the domain is
; default match is secure bit on and port 443
; so: Match = "domain.com" will work ^
; but you can explicitly set a match too
TitleId = "57520828"
Match = "https://ozzy360-wbid.live.ws.fireteam.net:443"
Redirect = "http://arkhamrevived.kiwifruitdev.com:80"

;add another entry like this:
;[NewEntry]
;TitleId = "title id in hex"
;...
```

If `DebuggerLogging` is set to `true`, you can see redirections through debugger output.

Tested using the [Cipher](https://cipher.services/) stealth service and [X360DebuggerWV](https://github.com/zeroKilo/X360DebuggerWV).

## Credits

- Byrom - [Halo Sunrise Plugin 2.0](https://github.com/Byrom90/Halo_Sunrise_Plugin_2.0)
- craftycodie - Halo hooks and addresses
- FreestyleDash Team - xkelib (modified to add xhttp connect function)

## Disclaimer

Some portions of the code in this repository was assisted with [GitHub Copilot](https://github.com/copilot).
