#include "GameUI.hpp"
#include "Hero.hpp"
#include "Scenes.hpp"

#include <iostream>

int main() {
    openGameWindow();

    Hero hero;
    clearScreen();
    hero.name = askName();

    int selected = choose("Choose your calling. Your calling changes where your story begins.", {
        "Explorer - begins in the Verdant-9 biosphere.",
        "Pilot - begins on Aramore Orbital Station.",
        "Voyager - begins in the Saltglass Nebula.",
        "Scholar - begins in the Obsidian Data Archive."},
        {ChoiceEffectiveness::Neutral, ChoiceEffectiveness::Neutral,
         ChoiceEffectiveness::Neutral, ChoiceEffectiveness::Neutral}, false);
    hero.calling = static_cast<Calling>(selected - 1);

    opening(hero);
    switch (hero.calling) {
        case Calling::Explorer: forestStart(hero); break;
        case Calling::Pilot: cityStart(hero); break;
        case Calling::Voyager: coastStart(hero); break;
        case Calling::Scholar: ruinsStart(hero); break;
    }

    bool routeToComms = extendedMission(hero);

    if (routeToComms && hero.health > 0 && commsArray(hero)) {
        finale(hero);
    } else if (!hero.gameEnded) {
        clearScreen();
        std::cout << "THE MISSION PAUSES\n\nYou awaken in an Aramore Station med bay, alive but changed. The relay still broadcasts beneath the station. Another mission awaits.\n";
    }

    std::cout << "\nThank you for playing.";
#ifdef _WIN32
    choose("Your mission has ended.", {"Close Game"}, {}, false);
#else
    std::cout << " Press Enter to close the mission.";
    std::cin.get();
#endif
    return 0;
}
