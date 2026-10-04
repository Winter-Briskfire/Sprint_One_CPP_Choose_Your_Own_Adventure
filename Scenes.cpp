#include "Scenes.hpp"

#include "GameUI.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {
using std::cout;
using std::string;
using std::vector;

string callingName(Calling calling) {
    switch (calling) {
        case Calling::Explorer: return "Explorer";
        case Calling::Pilot: return "Pilot";
        case Calling::Voyager: return "Voyager";
        default: return "Scholar";
    }
}

string advantageName(MissionAdvantage advantage) {
    switch (advantage) {
        case MissionAdvantage::Ally: return "Ally";
        case MissionAdvantage::Device: return "Device";
        case MissionAdvantage::Password: return "Password";
        case MissionAdvantage::Route: return "Route";
        case MissionAdvantage::Stealth: return "Stealth";
        case MissionAdvantage::Override: return "Override";
        default: return "None";
    }
}

string advantageAction(MissionAdvantage advantage) {
    switch (advantage) {
        case MissionAdvantage::Ally: return "Coordinate a signal strike with your ally.";
        case MissionAdvantage::Device: return "Use your recovered device to isolate the core.";
        case MissionAdvantage::Password: return "Broadcast the relay's original command.";
        case MissionAdvantage::Route: return "Use your mapped route to sever the relay quietly.";
        case MissionAdvantage::Stealth: return "Deploy your stealth plan inside the core chamber.";
        case MissionAdvantage::Override: return "Use the service-unit override against the crown.";
        default: return "Attempt an emergency relay shutdown.";
    }
}

void showAdvantageEnding(const Hero& hero) {
    switch (hero.advantage) {
        case MissionAdvantage::Ally:
            cout << "THE ALLIED SIGNAL ENDING\n\nYour ally holds the signal firewall while you disable Veyr's crown. The relay becomes a free navigation beacon, maintained by the people it once controlled.\n";
            break;
        case MissionAdvantage::Device:
            cout << "THE PRECISION ENDING\n\nYour recovered device isolates the hostile intelligence without destroying the relay. Starships keep their routes, and the core is placed under civilian control.\n";
            break;
        case MissionAdvantage::Password:
            cout << "THE REMEMBERED SKY ENDING\n\nThe original command restores the relay's forgotten safeguards. Veyr's crown goes dark, and the station receives a message from the system's first explorers.\n";
            break;
        case MissionAdvantage::Route:
            cout << "THE UNSEEN VECTOR ENDING\n\nYour route leads behind Veyr's defenses. You sever the control link without a fight, leaving the relay online and the sector free.\n";
            break;
        case MissionAdvantage::Stealth:
            cout << "THE GHOST PROTOCOL ENDING\n\nYour stealth plan lets you replace Veyr's command code before he realizes you are there. His crown powers down, and no one else is hurt.\n";
            break;
        case MissionAdvantage::Override:
            cout << "THE LIBERATED NETWORK ENDING\n\nYour override frees every service unit connected to the relay. Together, they reject Veyr's command and rebuild the station on their own terms.\n";
            break;
        default:
            cout << "THE EMERGENCY ENDING\n\nYou trigger a manual shutdown and escape as the relay falls silent. The sector is safe, but its future remains uncertain.\n";
    }
}

void showMissionAftermath(const Hero& hero) {
    if (hero.civilianTrust >= 3) {
        cout << "\nBecause you protected civilians during the mission, Aramore's residents organize to defend the station's future.\n";
    } else if (hero.civilianTrust < 0) {
        cout << "\nThe sector is safe, but survivors remember the people you chose not to help.\n";
    }
    if (hero.signalStability >= 3) {
        cout << "Your relay repairs hold. Safe star-lane navigation returns within days.\n";
    } else if (hero.signalStability < 0) {
        cout << "The damaged relay stays online only intermittently, leaving the sector with a difficult recovery.\n";
    }
}

void showStatus(const Hero& hero) {
    line();
    cout << hero.name << " the " << callingName(hero.calling)
         << "  |  Health: " << hero.health << "  Resolve: " << hero.resolve
         << "  Clues: " << hero.clues << "  Trust: " << hero.civilianTrust
         << "  Stability: " << hero.signalStability << "  Advantage: " << advantageName(hero.advantage) << "\n";
    line();
}

void showDecisionContinuity(const Hero& hero) {
    if (hero.lastDecision.empty()) return;

    cout << "MISSION CONTINUITY\nYou chose: " << hero.lastDecision << "\n";
    if (hero.alertRaised) cout << "That decision has left station security actively searching for you.\n";
    if (hero.hasDevice) cout << "Your recovered equipment is available for technical solutions.\n";
    if (hero.hasAlly) cout << "An ally can support choices that require coordination.\n";
    if (hero.knowsPassword) cout << "You have access to the relay's authentication knowledge.\n";
    cout << "\n";
}

string followUp(int previousChoice, const vector<string>& transitions, const string& scenario) {
    if (previousChoice >= 1 && previousChoice <= static_cast<int>(transitions.size())) {
        return transitions[previousChoice - 1] + "\n\n" + scenario;
    }
    return scenario;
}

int scenarioChoice(Hero& hero, const string& scenario, const vector<string>& choices,
                   const vector<ChoiceEffectiveness>& effectiveness = {}) {
    clearScreen();
    showStatus(hero);
    showDecisionContinuity(hero);
    cout << scenario << "\n";
    int selected = choose("What will you do?", choices, effectiveness);
    hero.lastDecision = choices[selected - 1];
    return selected;
}

void showEarlyEnding(Hero& hero, const string& title, const string& outcome) {
    hero.gameEnded = true;
    clearScreen();
    showStatus(hero);
    cout << title << "\n\n" << outcome << "\n";
}
}

void opening(const Hero& hero) {
    clearScreen();
    cout << "For a century, the Sunken Star relay has drifted beyond the Aramore system in silence. Tonight, it is broadcasting again. If its signal finishes uploading, every navigation network in the sector will fail.\n\n";
    cout << "You are " << hero.name << ", a " << callingName(hero.calling) << " selected by an encrypted emergency transmission.\n";
    pause();
}

void forestStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "VERDANT-9 BIOSPHERE\n\nYour shuttle touches down beneath towering violet canopy trees. A damaged survey drone projects a distress beacon beside a sealed research hatch. Something inside taps three times.\n";
    int path = choose("What does an Explorer do?", {"Follow the drone's hidden survey route.", "Force open the research hatch.", "Climb a canopy tower to scan the valley."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::Low, ChoiceEffectiveness::High});
    if (path == 1) {
        hero.lastDecision = "follow the drone's hidden survey route";
        hero.hasAlly = true; hero.advantage = MissionAdvantage::Ally; ++hero.clues;
        cout << "The drone leads you to a field scientist's shelter and uploads a star map marker for Aramore Station. The scientist joins your mission.\n";
    } else if (path == 2) {
        hero.lastDecision = "force open the research hatch";
        --hero.health; hero.hasDevice = true; hero.advantage = MissionAdvantage::Device; ++hero.clues;
        cout << "The hatch opens onto defensive bio-vines. You escape with a few cuts, but recover an access chip marked with the relay's symbol.\n";
    } else {
        hero.lastDecision = "climb the canopy tower to scan the valley";
        ++hero.resolve; hero.advantage = MissionAdvantage::Route;
        cout << "From the tower, you spot a debris field above Aramore Station and plot a safer approach vector. Your confidence hardens.\n";
    }
    pause();
}

void cityStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "ARAMORE ORBITAL STATION\n\nYou arrive during a cargo transfer. The docking ring is sealing, and an anxious systems engineer presses an access key into your palm. Security drones are scanning every passenger.\n";
    int path = choose("What does a Pilot do?", {"Talk your way through the docking inspection.", "Hide inside a freight container.", "Follow the engineer into the service level."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::Low, ChoiceEffectiveness::High});
    if (path == 1) {
        hero.lastDecision = "talk your way through the docking inspection";
        ++hero.resolve; ++hero.clues; hero.advantage = MissionAdvantage::Route;
        cout << "A calm explanation and a well-timed rumor convince the docking chief to clear you. She quietly tells you that the command deck was abandoned.\n";
    } else if (path == 2) {
        hero.lastDecision = "hide inside a freight container";
        --hero.health; ++hero.clues; hero.advantage = MissionAdvantage::Stealth;
        cout << "The coolant fumes make you sneeze at the worst possible moment. You escape a short chase, carrying a note that reads: 'Below the comms array.'\n";
    } else {
        hero.lastDecision = "follow the engineer into the service level";
        hero.hasAlly = true; hero.advantage = MissionAdvantage::Ally;
        cout << "The engineer reveals herself as a resistance courier. She gives you the access key and promises to meet you beneath the comms array.\n";
    }
    pause();
}

void coastStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE SALTGLASS NEBULA\n\nA derelict freighter emerges from a mirror-bright dust cloud. Its emergency beacon repeats inside the hull while a salvage pilot begs you not to answer it. An ion storm is closing in quickly.\n";
    int path = choose("What does a Voyager do?", {"Board the freighter and disable the beacon.", "Question the frightened salvage pilot.", "Use a sensor sweep to map the debris field."}, {ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High, ChoiceEffectiveness::High});
    if (path == 1) {
        hero.lastDecision = "board the freighter and disable the beacon";
        hero.hasDevice = true; hero.advantage = MissionAdvantage::Device;
        cout << "Inside, you find a quantum compass that points toward the relay signal. The beacon goes quiet, and the ion storm begins to disperse.\n";
    } else if (path == 2) {
        hero.lastDecision = "question the frightened salvage pilot";
        hero.clues += 2; hero.knowsPassword = true; hero.advantage = MissionAdvantage::Password;
        cout << "The pilot admits that he carried a masked stranger to the relay access point. The stranger's password was: 'The sky remembers.'\n";
    } else {
        hero.lastDecision = "use a sensor sweep to map the debris field";
        ++hero.resolve; hero.advantage = MissionAdvantage::Route;
        cout << "Your sweep reveals an ancient jump corridor through the debris. You take a shard of encoded star-metal as an electromagnetic shield.\n";
    }
    pause();
}

void ruinsStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE OBSIDIAN DATA ARCHIVE\n\nDust drifts through servers older than the colony. A luminous data core cycles through its own files while a sealed observatory tracks a point beyond the moonless horizon.\n";
    int path = choose("What does a Scholar do?", {"Read the luminous data core.", "Align the observatory array.", "Search the restricted servers for a weapon."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::High, ChoiceEffectiveness::Low});
    if (path == 1) {
        hero.lastDecision = "read the luminous data core";
        hero.knowsPassword = true; ++hero.clues; hero.advantage = MissionAdvantage::Password;
        cout << "The data core reveals the relay's original command: 'The sky remembers.' It warns that force strengthens the hostile intelligence below.\n";
    } else if (path == 2) {
        hero.lastDecision = "align the observatory array";
        hero.hasDevice = true; hero.advantage = MissionAdvantage::Device;
        cout << "A laser alignment opens a concealed compartment containing a prism scanner. It can reveal the source code behind any projected illusion.\n";
    } else {
        hero.lastDecision = "search the restricted servers for a weapon";
        --hero.health; ++hero.resolve; hero.advantage = MissionAdvantage::Override;
        cout << "A security hologram objects to your methods. You defeat it and recover its plasma cutter, but not without a painful burn.\n";
    }
    pause();
}

bool extendedMission(Hero& hero) {
    int choice = scenarioChoice(hero, "THE LONG APPROACH\n\nA civilian transport sends a distress call from a failing jump gate.", {"Stop and evacuate the passengers.", "Transmit repair instructions and continue.", "Salvage its navigation core."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::Neutral, ChoiceEffectiveness::Low});
    if (choice == 1) { ++hero.civilianTrust; cout << "You lose time, but the passengers broadcast your rescue across the sector.\n"; }
    else if (choice == 2) { ++hero.clues; cout << "Your remote repair works. The captain sends you gate telemetry that matches the relay signal.\n"; }
    else { hero.hasDevice = true; --hero.civilianTrust; cout << "You recover a navigation core, but the stranded crew remembers your decision.\n"; }
    pause();

    choice = scenarioChoice(hero, followUp(choice, {
        "The rescued passengers relay your name to a nearby patrol, which arrives asking why a civilian transport has praised an unauthorized operative.",
        "The repair telemetry you transmitted is intercepted by a patrol frigate, which demands the credentials behind your relay access.",
        "The navigation core you salvaged carries a tracked station signature. A patrol frigate intercepts you to reclaim it."},
        "A patrol frigate demands your mission credentials."), {"Tell the truth about the relay.", "Present forged clearance.", "Slip through an asteroid field."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::Low, ChoiceEffectiveness::Neutral});
    if (choice == 1) { ++hero.civilianTrust; ++hero.resolve; cout << "The captain cannot help openly, but sends a quiet warning about Veyr's security force.\n"; }
    else if (choice == 2) { hero.alertRaised = true; cout << "The forgery works, but the patrol flags your transponder for later review.\n"; }
    else { ++hero.signalStability; cout << "You evade the patrol and chart a clean, low-interference route to the station.\n"; }
    pause();

    choice = scenarioChoice(hero, followUp(choice, {
        "The patrol captain quietly forwards a classified location from Veyr's security file: a research vessel drifting ahead of you.",
        "Your forged clearance gives you a temporary route through a quarantined research zone, where a derelict vessel is broadcasting.",
        "Your asteroid route brings you alongside an unlisted research vessel hidden in the debris."},
        "The vessel repeats a fragment of the Sunken Star signal."), {"Decode the fragment from a distance.", "Board the vessel to retrieve its core.", "Destroy the transmitter before it spreads."}, {ChoiceEffectiveness::High, (hero.hasDevice || (hero.knowsPassword && hero.clues >= 2 && hero.signalStability > 0)) ? ChoiceEffectiveness::High : ChoiceEffectiveness::Low, ChoiceEffectiveness::Low});
    if (choice == 1) { ++hero.clues; ++hero.signalStability; cout << "The fragment reveals a relay failsafe and strengthens your understanding of the core.\n"; }
    else if (choice == 2) { --hero.health; hero.hasDevice = true; cout << "You retrieve a diagnostic core, but radiation leaks through your suit seal.\n"; }
    else { --hero.civilianTrust; cout << "The signal stops, but nearby scavengers accuse you of destroying valuable evidence.\n"; }
    pause();

    // An exceptionally prepared operative can use the vessel's failsafe before
    // the mission reaches Aramore. This creates a legitimate short success route.
    if (hero.knowsPassword && hero.hasDevice && hero.clues >= 2 && hero.signalStability > 0) {
        showEarlyEnding(hero, "THE REMOTE FAILSAFE ENDING",
            "Your relay password, diagnostic device, and clean flight path form a complete authentication chain. From the research vessel, you activate the Sunken Star's original failsafe before Veyr can react. The hostile upload collapses, and Aramore wakes to open star lanes.");
        return false;
    }

    // Three reckless choices in a row create a short, failed route instead of
    // pretending that every decision leads to the same long mission.
    if (hero.civilianTrust < 0 && hero.alertRaised && choice == 3) {
        showEarlyEnding(hero, "THE INTERCEPTED SIGNAL ENDING",
            "The stranded transport reports your salvage, the patrol recognizes the forged clearance, and the destroyed transmitter leaves no cover. Veyr's frigate locks onto your signal and forces you to retreat. The relay remains in enemy hands.");
        return false;
    }

    choice = scenarioChoice(hero, followUp(choice, {
        "The decoded fragment contains an emergency route, but a rogue broadcaster claims it is unsafe and offers a different passage.",
        "The diagnostic core you recovered wakes a rogue transmitter that recognizes its research-vessel signature.",
        "Destroying the transmitter leaves a gap in local navigation. A rogue broadcaster fills it with an offer of safe passage."},
        "It asks for your route data in exchange for a way to Aramore."), {"Share a limited route map.", "Trace the broadcast before replying.", "Reject the offer and travel alone."}, {ChoiceEffectiveness::High, ChoiceEffectiveness::High, ChoiceEffectiveness::Neutral});
    if (choice == 1) { ++hero.civilianTrust; cout << "Independent pilots begin using your route and promise to support you if the station falls.\n"; }
    else if (choice == 2) { ++hero.clues; hero.knowsPassword = true; cout << "You trace the signal to a former relay engineer and receive a second authentication phrase.\n"; }
    else { ++hero.resolve; cout << "You avoid a possible trap, but arrive without outside support.\n"; }
    pause();

    bool fastApproach = hero.advantage == MissionAdvantage::Route || hero.advantage == MissionAdvantage::Stealth;
    if (fastApproach) {
        cout << "Your earlier route planning bypasses the damaged outer ring. You reach the lower station before the maintenance emergency and resistance diversion unfold.\n";
    } else {
        choice = scenarioChoice(hero, "UNDER ARAMORE\n\nA maintenance crew is trapped behind a decompression door.", {"Override the door and rescue them.", "Guide them through a remote airlock cycle.", "Leave before security notices you."}, {hero.health > 1 ? ChoiceEffectiveness::High : ChoiceEffectiveness::Low, ChoiceEffectiveness::High, ChoiceEffectiveness::Low});
        if (choice == 1) { ++hero.civilianTrust; --hero.health; cout << "You pull the crew to safety, but a pressure burst damages your suit.\n"; }
        else if (choice == 2) { ++hero.clues; cout << "The crew escapes and gives you a maintenance schematic of the lower station.\n"; }
        else { hero.alertRaised = true; cout << "You remain unseen, but the abandoned crew triggers an alarm that reaches station security.\n"; }

        choice = scenarioChoice(hero, "A resistance cell asks you to choose a target for its diversion.", {"Divert security drones away from the relay.", "Disable the command-deck sensors.", "Protect the civilian docking ring."}, {ChoiceEffectiveness::High, hero.alertRaised ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High});
        if (choice == 1) { hero.hasAlly = true; cout << "The cell assigns a technician to support your approach to the relay.\n"; }
        else if (choice == 2) { hero.alertRaised = false; ++hero.resolve; cout << "The sensors go dark, erasing the alert attached to your transponder.\n"; }
        else { hero.civilianTrust += 2; cout << "The docking ring survives the attack, and civilians begin organizing a station-wide signal firewall.\n"; }
    }

    choice = scenarioChoice(hero, "A sealed lab contains an experimental relay stabilizer.", {"Install the stabilizer in your equipment.", "Copy its research logs.", "Leave it sealed; it may be dangerous."}, {hero.hasDevice ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High, ChoiceEffectiveness::Neutral});
    if (choice == 1) { hero.hasDevice = true; hero.signalStability += 2; cout << "The stabilizer synchronizes with your gear and can protect the core from overload.\n"; }
    else if (choice == 2) { hero.clues += 2; cout << "The logs explain how Veyr's crown manipulates navigation data.\n"; }
    else { ++hero.resolve; cout << "You avoid an unstable prototype, but sacrifice a possible technical advantage.\n"; }
    pause();

    choice = scenarioChoice(hero, "Veyr broadcasts that all unauthorized operatives will be expelled into space.", {"Answer with a public counter-broadcast.", "Jam the message quietly.", "Use it to locate Veyr's command signal."}, {hero.civilianTrust > 0 ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High, ChoiceEffectiveness::High});
    if (choice == 1) { ++hero.civilianTrust; hero.alertRaised = true; cout << "Your response inspires the station, but Veyr now knows your voice and location.\n"; }
    else if (choice == 2) { hero.alertRaised = false; cout << "The message cuts out. Security remains uncertain about where you are.\n"; }
    else { ++hero.clues; hero.knowsPassword = true; cout << "The carrier wave exposes Veyr's private command channel and confirms the relay protocol.\n"; }
    pause();

    if (hero.civilianTrust >= 2) {
        choice = scenarioChoice(hero, "Civilians you helped offer to secure one part of the station.", {"Ask them to protect the medical bays.", "Ask them to reinforce the signal firewall.", "Ask them to evacuate the docking ring."}, {hero.health < 3 ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High, ChoiceEffectiveness::High});
        if (choice == 1) { ++hero.health; cout << "Station medics patch your suit and send supplies to the lower levels.\n"; }
        else if (choice == 2) { ++hero.signalStability; cout << "The civilian firewall reduces interference around the relay core.\n"; }
        else { ++hero.civilianTrust; cout << "The docking ring empties safely, giving the station a clearer evacuation route.\n"; }
    }

    if (hero.alertRaised) {
        choice = scenarioChoice(hero, "Veyr's alert triggers a lockdown between you and the core.", {"Cut power to the lockdown doors.", "Create a decoy transmission.", "Force a way through."}, {ChoiceEffectiveness::High, hero.signalStability > 0 ? ChoiceEffectiveness::Neutral : ChoiceEffectiveness::Low, ChoiceEffectiveness::Low});
        if (choice == 1) { hero.alertRaised = false; ++hero.clues; cout << "The doors open, and you recover a security map before moving on.\n"; }
        else if (choice == 2) { hero.alertRaised = false; --hero.signalStability; cout << "Security follows your decoy, but the transmission destabilizes the relay signal.\n"; }
        else { --hero.health; cout << "You force the door, reaching the core with a damaged suit.\n"; }
    }

    choice = scenarioChoice(hero, "DESCENT TO THE RELAY\n\nThe cargo lift has room for one extra item.", {"Bring emergency medical supplies.", "Bring a portable power cell.", "Bring a drone-control relay."}, {hero.health < 3 ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, ChoiceEffectiveness::High, hero.hasAlly ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral});
    if (choice == 1) { ++hero.civilianTrust; ++hero.health; cout << "You treat your injuries and leave supplies for station workers.\n"; }
    else if (choice == 2) { ++hero.signalStability; cout << "The power cell gives you a controlled shutdown option at the core.\n"; }
    else { hero.hasAlly = true; cout << "You take command of a maintenance drone that can assist during the confrontation.\n"; }
    pause();

    choice = scenarioChoice(hero, "Security drones block the reactor corridor.", {"Use a distraction to draw them away.", "Disable them with your equipment.", "Find a ventilation route."}, {ChoiceEffectiveness::Neutral, hero.hasDevice ? ChoiceEffectiveness::High : ChoiceEffectiveness::Low, hero.advantage == MissionAdvantage::Stealth ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral});
    if (choice == 1) { hero.alertRaised = true; ++hero.resolve; cout << "The drones pursue your decoy, but Veyr receives an urgent security report.\n"; }
    else if (choice == 2 && hero.hasDevice) { ++hero.clues; cout << "Your device disables the drones and downloads their access records.\n"; }
    else if (choice == 2) { --hero.health; cout << "Without the right equipment, disabling the drones costs you a painful shock.\n"; }
    else { hero.alertRaised = false; ++hero.resolve; cout << "The ventilation route keeps you unseen and clears the security alert from your approach.\n"; }
    pause();

    choice = scenarioChoice(hero, "You find a recorded message from the relay's original engineers.", {"Watch the full recording.", "Extract only the emergency command.", "Erase it so Veyr cannot recover it."}, {ChoiceEffectiveness::High, hero.knowsPassword ? ChoiceEffectiveness::Neutral : ChoiceEffectiveness::High, ChoiceEffectiveness::Low});
    if (choice == 1) { hero.clues += 2; ++hero.civilianTrust; cout << "The engineers explain why the relay must remain accountable to the people who use it.\n"; }
    else if (choice == 2) { hero.knowsPassword = true; cout << "You secure a direct shutdown command, but lose the broader history.\n"; }
    else { --hero.signalStability; cout << "Veyr loses the evidence, but so do you. The relay becomes harder to restore safely.\n"; }
    pause();

    choice = scenarioChoice(hero, "The core's outer shield begins to fail.", {"Stabilize it before entering.", "Rush through before it collapses.", "Redirect power to protect the station."}, {hero.hasDevice ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral, hero.health > 1 ? ChoiceEffectiveness::Neutral : ChoiceEffectiveness::Low, hero.civilianTrust > 0 ? ChoiceEffectiveness::High : ChoiceEffectiveness::Neutral});
    if (choice == 1) { hero.signalStability += 2; cout << "The shield steadies. You enter a calmer core chamber with more options available.\n"; }
    else if (choice == 2) { --hero.health; ++hero.resolve; cout << "You make it through the surge, but your suit takes another hit.\n"; }
    else { hero.civilianTrust += 2; --hero.signalStability; cout << "The station survives the surge, though the relay core becomes more volatile.\n"; }
    pause();
    return true;
}

bool commsArray(Hero& hero) {
    clearScreen();
    showStatus(hero);
    cout << "THE COMMS ARRAY\n\nAll routes lead to Aramore's silent communications array. Its access panel shows an eye, a hand, and a wave. A masked security android steps from the shadows: 'Name the memory that outlives the stars.'\n";
    if (hero.alertRaised) cout << "The android projects your flagged transponder ID. Veyr's security network is ready for you.\n";
    if (hero.civilianTrust >= 3) { hero.hasAlly = true; cout << "Station civilians send a maintenance drone to assist you because of the people you helped.\n"; }
    vector<string> responses;
    vector<int> responseCodes;
    vector<ChoiceEffectiveness> responseEffectiveness;
    if (hero.knowsPassword) {
        responses.push_back("Use the relay password you discovered earlier.");
        responseCodes.push_back(1);
        responseEffectiveness.push_back(ChoiceEffectiveness::High);
    }
    if (hero.hasDevice) {
        responses.push_back("Offer the device you recovered earlier.");
        responseCodes.push_back(2);
        responseEffectiveness.push_back(ChoiceEffectiveness::High);
    }
    if (hero.hasAlly || hero.advantage == MissionAdvantage::Ally || hero.advantage == MissionAdvantage::Override) {
        responses.push_back("Coordinate with the ally you gained earlier.");
        responseCodes.push_back(3);
        responseEffectiveness.push_back(ChoiceEffectiveness::High);
    }
    if (hero.clues >= 2 || hero.advantage == MissionAdvantage::Route || hero.advantage == MissionAdvantage::Stealth) {
        responses.push_back("Use the route information you gathered earlier.");
        responseCodes.push_back(4);
        responseEffectiveness.push_back(ChoiceEffectiveness::High);
    }

    int selectedResponse = choose("Choose an unlocked response.", responses, responseEffectiveness);
    int action = responseCodes[selectedResponse - 1];
    if (action == 1) {
        cout << "'The sky remembers.' The android stands aside and opens the access hatch.\n";
        ++hero.clues;
    } else if (action == 2) {
        cout << "The android accepts your device, then returns it recalibrated: it now glows near the relay. You are granted access.\n";
        ++hero.resolve;
    } else if (action == 3) {
        hero.hasAlly = true;
        cout << "Your earlier choice pays off. Your ally disables the android's control mask before it can strike. The freed service unit opens the way.\n";
    } else {
        cout << "Your earlier route planning pays off. The symbols reveal a maintenance conduit, allowing you to bypass the android and reach the lift unseen.\n";
    }
    pause();
    return hero.health > 0;
}

void finale(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE SUNKEN STAR RELAY\n\nFar below the station, a blue quantum core hangs above a bottomless reactor shaft. Director Veyr stands before it, wearing a black-glass neural crown. 'One perfect route,' he says, 'and no ship will ever be lost again.'\n\nThe core pulses. Every future you might choose flickers in its light.\n";
    ChoiceEffectiveness advantageRating = hero.advantage == MissionAdvantage::None ? ChoiceEffectiveness::Neutral : ChoiceEffectiveness::High;
    ChoiceEffectiveness crownRating = (hero.resolve >= 3 || hero.hasAlly) ? ChoiceEffectiveness::High : ChoiceEffectiveness::Low;
    ChoiceEffectiveness systemRating = hero.civilianTrust < 0 ? ChoiceEffectiveness::Neutral : ChoiceEffectiveness::Low;
    ChoiceEffectiveness shutdownRating = hero.signalStability < 0 ? ChoiceEffectiveness::Low : ChoiceEffectiveness::Neutral;
    int ending = choose("What will you do?", {advantageAction(hero.advantage), "Destroy the neural crown.", "Use the relay to create one perfect system.", "Shut down the relay forever."}, {advantageRating, crownRating, systemRating, shutdownRating});
    clearScreen();
    if (ending == 1) showAdvantageEnding(hero);
    else if (ending == 2 && (hero.resolve >= 3 || hero.hasAlly)) cout << "THE OPEN SKIES ENDING\n\nWith your ally holding the signal firewall, you shatter the crown. Veyr collapses, weeping, and the core becomes a thousand harmless lights. The star lanes return: messy, dangerous, and gloriously free.\n";
    else if (ending == 2) cout << "THE LAST TRANSMISSION ENDING\n\nThe crown breaks, but its fragments overload your suit. You save the sector as the relay's final transmission carries your name into legend.\n";
    else if (ending == 3) cout << "THE GOLDEN CAGE ENDING\n\nFor one shining year, no pilot is lost and no choice hurts. Then the people of Aramore begin dreaming of jump gates they cannot open. You govern a perfect system and wonder whether it is still alive.\n";
    else cout << "THE QUIET ORBIT ENDING\n\nYou shut down the relay with every unit of power you can spare. The sector is safe, though its old wonders fade. Years later, children still tell stories about the " << callingName(hero.calling) << " who chose a quiet orbit.\n";
    showMissionAftermath(hero);
}
