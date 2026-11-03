#include "string"
#include "vector"
#include "stdafx.h"
#include "TrustCheck.h"

bool TrustCheck::trustCheck(std::string clientVersion)
{
    std::string allowedVersions[] = { "2016E", "2017L" };
    auto it = std::find(std::begin(allowedVersions), std::end(allowedVersions), clientVersion);
    return it != std::end(allowedVersions);
}