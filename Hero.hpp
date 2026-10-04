#pragma once

#include <string>

enum class Calling { Explorer, Pilot, Voyager, Scholar };
enum class MissionAdvantage { None, Ally, Device, Password, Route, Stealth, Override };

class Hero {
public:
    std::string name;
    Calling calling = Calling::Explorer;
    int health = 3;
    int resolve = 2;
    int clues = 0;
    int civilianTrust = 0;
    int signalStability = 0;
    bool hasDevice = false;
    bool hasAlly = false;
    bool knowsPassword = false;
    bool alertRaised = false;
    bool gameEnded = false;
    std::string lastDecision;
    MissionAdvantage advantage = MissionAdvantage::None;
};
