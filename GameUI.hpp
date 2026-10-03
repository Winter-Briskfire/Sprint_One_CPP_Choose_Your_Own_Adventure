#pragma once

#include <string>
#include <vector>

void openGameWindow();
void clearScreen();
void line();
void pause();
enum class ChoiceEffectiveness { High, Neutral, Low };

int choose(const std::string& question, const std::vector<std::string>& choices,
           const std::vector<ChoiceEffectiveness>& effectiveness = {}, bool showIndicators = true);
std::string askName();
