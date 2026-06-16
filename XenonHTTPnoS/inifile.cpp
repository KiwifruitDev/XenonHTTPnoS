#include "stdafx.h"
#include "inifile.h"
#include "util.h"

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>

struct RuleHost
{
    std::string host;
    WORD port;
    bool secure;
};

struct RedirectRule
{
    int titleId;
    RuleHost match;
    RuleHost redirect;
};

static std::vector<RedirectRule> g_redirectRules;
BOOL bDebuggerLogging = TRUE; // output debug info until ini is loaded

static const char defaultIni[] =
"[Options]\r\n"
"DebuggerLogging = false\r\n"
"\r\n"
"[ArkhamRevived]\r\n"
"; urls are not directly replaced, only the domain is\r\n"
"; default match is secure bit on and port 443\r\n"
"; so: Match = \"domain.com\" will work ^\r\n"
"; but you can explicitly set a match too\r\n"
"TitleId = \"57520828\"\r\n"
"Match = \"https://ozzy360-wbid.live.ws.fireteam.net:443\"\r\n"
"Redirect = \"http://arkhamrevived.kiwifruitdev.com:80\"\r\n";

static std::string Trim(const std::string& value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace((unsigned char)value[start]))
        start++;

    size_t end = value.size();
    while (end > start && std::isspace((unsigned char)value[end - 1]))
        end--;

    return value.substr(start, end - start);
}

static std::string StripQuotes(const std::string& value)
{
    std::string trimmed = Trim(value);
    if (trimmed.size() >= 2)
    {
        char first = trimmed.front();
        char last = trimmed.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
            return trimmed.substr(1, trimmed.size() - 2);
    }
    return trimmed;
}

static std::string ToLower(const std::string& value)
{
    std::string result = value;
    for (std::string::size_type i = 0; i < result.size(); ++i)
    {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

static bool IsNumber(const std::string& value)
{
    if (value.empty())
        return false;
    for (std::string::size_type i = 0; i < value.size(); ++i)
    {
        char c = value[i];
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

static bool ParseHost(const char* rawValue, RuleHost& rule)
{
    // parse the ini url
    std::string data = StripQuotes(rawValue ? rawValue : "");
    if (data.empty())
        return false;
    std::string remainder = data;

    // get scheme/protocol (http or https)
    std::string scheme;
    size_t schemePos = data.find("://");
    if (schemePos != std::string::npos)
    {
        scheme = ToLower(data.substr(0, schemePos));
        remainder = data.substr(schemePos + 3);
    }

    // trimming ://
    size_t slashPos = remainder.find('/');
    if (slashPos != std::string::npos)
        remainder = remainder.substr(0, slashPos);

    // make sure there is still a domain after trimming
    remainder = Trim(remainder);
    if (remainder.empty())
        return false;

    // port can be specified with : but it is optional
    bool hasPort = false;
    WORD port = 0;
    size_t colonPos = remainder.rfind(':');
    if (colonPos != std::string::npos && colonPos > 0)
    {
        std::string portText = remainder.substr(colonPos + 1);
        if (IsNumber(portText))
        {
            int parsed = std::atoi(portText.c_str());
            if (parsed > 0 && parsed <= 65535)
            {
                hasPort = true;
                port = static_cast<WORD>(parsed);
                // remove : and remaining, this should be the domain/host now
                remainder = Trim(remainder.substr(0, colonPos));
            }
        }
    }

    // make sure there is still a domain after trimming
    if (remainder.empty())
        return false;

    // the secure bit is matched using http/https
    // this is done to make the ini more human-readable and compact
    // other schemes are not supported, default to matching https
    bool secure = true;
    if (!scheme.empty())
    {
        secure = (scheme == "https");
    }

    // infer depending on whether secure bit is set or not
    if (!hasPort)
    {
        port = secure ? 443 : 80;
    }

    rule.host = ToLower(remainder);
    rule.port = port;
    rule.secure = secure;
    return true;
}

static void LoadRedirectRules(CSimpleIniA& ini)
{
    g_redirectRules.clear();

    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);

    for (CSimpleIniA::TNamesDepend::const_iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        const CSimpleIniA::Entry& entry = *iter;
        if (!entry.pItem)
            continue;

        const char* section = entry.pItem;
        
        std::string sectionName = ToLower(section);
        if (sectionName == "options")
            continue;
        
        const char* titleId = ini.GetValue(section, "TitleId", NULL);
        const char* match = ini.GetValue(section, "Match", NULL);
        const char* redirect = ini.GetValue(section, "Redirect", NULL);
        if (!titleId || !match || !redirect)
            continue;

        // parse titleid as hex characters
        int iTitleId;
        std::string titleIdStr = StripQuotes(titleId);
        try
        {
            if (titleIdStr.size() > 8)
                throw std::out_of_range("TitleId too long");
            iTitleId = static_cast<int>(std::stoul(titleIdStr, NULL, 16));
        }
        catch (const std::invalid_argument& _)
        {
            DebugPrint("Invalid TitleId in section %s! (must be hex in quotes)\n", section);
            continue;
        } 
        catch (const std::out_of_range& _)
        {
            DebugPrint("Invalid TitleId in section %s! (must be hex in quotes)\n", section);
            continue;
        }
        
        RedirectRule rule;
        rule.titleId = iTitleId;

        if (!ParseHost(match, rule.match))
        {
            DebugPrint("Invalid Match in section %s!\n", section);
            continue;
        }
        if (!ParseHost(redirect, rule.redirect))
        {
            DebugPrint("Invalid Redirect in section %s!\n", section);
            continue;
        }

        g_redirectRules.push_back(rule);
        DebugPrint("Loaded %s redirect rule for title %s: %s://%s:%d -> %s://%s:%d\n",
            section,
            titleIdStr.c_str(),
            rule.match.secure ? "https" : "http",
            rule.match.host.c_str(),
            rule.match.port,
            rule.redirect.secure ? "https" : "http",
            rule.redirect.host.c_str(),
            rule.redirect.port
        );
    }
}

static void EnsureDefaultOptions(CSimpleIniA& ini)
{
    const char* current = ini.GetValue("Options", "DebuggerLogging", NULL);
    if (!current)
    {
        ini.SetBoolValue("Options", "DebuggerLogging", FALSE);
    }
}

// Save changes to (use false) or generate new (use true) XenonHTTPnoS.ini
VOID Writeini(BOOL GenerateNew)
{
    if (GenerateNew)
    {
        if (CWriteFile(PATH_INI, defaultIni, sizeof(defaultIni)) == TRUE)
        {
            DebugPrint("XenonHTTPnoS.ini created successfully!\n");
            Readini();
        }
        else
        {
            DebugPrint("Failed to create XenonHTTPnoS.ini!\n");
        }
        return;
    }

    CSimpleIniA ini_W;
    ini_W.Reset();
    ini_W.SetSpaces(true);
    if (ini_W.LoadFile(PATH_INI) == SI_OK)
    {
        ini_W.SetBoolValue("Options", "DebuggerLogging", bDebuggerLogging);
        if (ini_W.SaveFile(PATH_INI) != SI_OK)
        {
            DebugPrint("Failed to save changes to XenonHTTPnoS.ini!\n");
        }
    }
    else
    {
        DebugPrint("Failed to load XenonHTTPnoS.ini for saving.\n");
    }
}

// Read ini values
VOID Readini()
{
    DebugPrint("Reading values from XenonHTTPnoS.ini...\n");
    CSimpleIniA ini_R;
    ini_R.Reset();
    ini_R.SetSpaces(true);

    if (ini_R.LoadFile(PATH_INI) == SI_OK)
    {
        bDebuggerLogging = ini_R.GetBoolValue("Options", "DebuggerLogging", FALSE);
        EnsureDefaultOptions(ini_R);
        LoadRedirectRules(ini_R);
        Writeini(FALSE);
    }
    else
    {
        DebugPrint("Error reading XenonHTTPnoS.ini. Generating new ini...\n");
        Writeini(TRUE);
    }
}

// Is this title ID specified by the ini?
BOOL MatchTitleID(DWORD titleId)
{
    for (std::vector<RedirectRule>::const_iterator iter = g_redirectRules.begin(); iter != g_redirectRules.end(); ++iter)
    {
        const RedirectRule& rule = *iter;
        if (rule.titleId == titleId)
            return TRUE;
    }
    return FALSE;
}

VOID RewriteHttpRequest(
    CONST CHAR* pcszServerName,
    WORD nServerPort,
    DWORD dwFlags,
    const CHAR** pNewServerName,
    WORD* pNewServerPort,
    DWORD* pNewFlags)
{
    if (!pcszServerName || !pNewServerName || !pNewServerPort || !pNewFlags)
        return;

    *pNewServerName = pcszServerName;
    *pNewServerPort = nServerPort;
    *pNewFlags = dwFlags;

    bool secureRequest = (dwFlags & XHTTP_FLAG_SECURE) != 0;

	DebugPrint("Match: %s://%s:%d\n", secureRequest ? "https" : "http", pcszServerName, nServerPort);

    std::string host = ToLower(Trim(pcszServerName));
    for (std::vector<RedirectRule>::const_iterator iter = g_redirectRules.begin(); iter != g_redirectRules.end(); ++iter)
    {
        const RedirectRule& rule = *iter;
        if (host != rule.match.host)
            continue;

        if (nServerPort != rule.match.port)
            continue;

        // validate secure bit
        if (secureRequest != rule.match.secure)
            continue;

        *pNewServerName = rule.redirect.host.c_str();
        *pNewServerPort = rule.redirect.port;

        // add or remove the secure bit (https)
        if (rule.redirect.secure)
            *pNewFlags |= XHTTP_FLAG_SECURE;
        else
            *pNewFlags &= ~XHTTP_FLAG_SECURE;

	    DebugPrint("Redirect: %s://%s:%d\n", rule.redirect.secure ? "https" : "http", rule.redirect.host.c_str(), rule.redirect.port);
        return;
    }
}
