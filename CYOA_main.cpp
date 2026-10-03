#include <cstdlib>
#include <ctime>
#include <streambuf>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using std::cin;
using std::cout;
using std::string;
using std::vector;

#ifdef _WIN32
// Native GUI support.  The story still uses cout, but this stream buffer sends
// that text to the game's own window instead of to Command Prompt.
HWND gameWindow = nullptr;
HWND storyBox = nullptr;
HFONT orbitron = nullptr;
HFONT titleFont = nullptr;
HBRUSH backgroundBrush = CreateSolidBrush(RGB(10, 16, 31));
std::string storyText;

void refreshStory() {
    if (storyBox) {
        SetWindowTextA(storyBox, storyText.c_str());
        SendMessageA(storyBox, EM_SETSEL, static_cast<WPARAM>(storyText.size()), static_cast<LPARAM>(storyText.size()));
        SendMessageA(storyBox, EM_SCROLLCARET, 0, 0);
    }
}

class StoryBuffer : public std::streambuf {
protected:
    int overflow(int character) override {
        if (character != EOF) {
            if (character == '\n' && (!storyText.empty() && storyText.back() != '\r')) {
                storyText.push_back('\r');
            }
            storyText.push_back(static_cast<char>(character));
            refreshStory();
        }
        return character;
    }

    std::streamsize xsputn(const char* text, std::streamsize count) override {
        std::string buffer(text, static_cast<size_t>(count));
        std::string converted;
        converted.reserve(buffer.size() + 8);
        for (size_t i = 0; i < buffer.size(); ++i) {
            if (buffer[i] == '\n' && (converted.empty() || converted.back() != '\r')) {
                converted.push_back('\r');
            }
            converted.push_back(buffer[i]);
        }
        storyText.append(converted);
        refreshStory();
        return count;
    }
};

StoryBuffer storyBuffer;
enum class UiMode { None, Name, Choices };
UiMode uiMode = UiMode::None;
vector<HWND> uiControls;
HWND nameField = nullptr;
string enteredName;
int chosenAnswer = 1;

void layoutGameWindow() {
    if (!gameWindow || !storyBox) return;
    RECT area{}; GetClientRect(gameWindow, &area);
    int width = area.right - area.left;
    int height = area.bottom - area.top;
    int storyBottom = height - 24;
    if (uiMode == UiMode::Choices) {
        int buttonCount = static_cast<int>(uiControls.size()) - 1;
        int top = height - (62 + buttonCount * 52);
        storyBottom = top - 12;
        MoveWindow(uiControls[0], 24, top, width - 48, 44, TRUE);
        for (int i = 0; i < buttonCount; ++i)
            MoveWindow(uiControls[i + 1], 24, top + 48 + i * 52, width - 48, 44, TRUE);
    } else if (uiMode == UiMode::Name) {
        int top = height - 168;
        storyBottom = top - 12;
        MoveWindow(uiControls[0], 24, top, width - 48, 36, TRUE);
        MoveWindow(nameField, 24, top + 42, width - 48, 36, TRUE);
        MoveWindow(uiControls[1], 24, top + 92, 240, 42, TRUE);
    }
    MoveWindow(storyBox, 24, 118, width - 48, (storyBottom > 220 ? storyBottom : 220) - 118, TRUE);
}

void clearInteractiveControls() {
    for (HWND control : uiControls) DestroyWindow(control);
    if (nameField) DestroyWindow(nameField);
    uiControls.clear();
    nameField = nullptr;
    uiMode = UiMode::None;
    layoutGameWindow();
}

LRESULT CALLBACK GameWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            storyBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                0, 0, 0, 0, window, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessageW(storyBox, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
            return 0;
        case WM_SIZE: layoutGameWindow(); return 0;
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            HDC device = BeginPaint(window, &paint);
            RECT area{}; GetClientRect(window, &area);
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(97, 204, 255));
            HGDIOBJ oldPen = SelectObject(device, pen);
            MoveToEx(device, 24, 22, nullptr); LineTo(device, area.right - 24, 22);
            MoveToEx(device, 24, 29, nullptr); LineTo(device, area.right - 24, 29);
            MoveToEx(device, 24, 93, nullptr); LineTo(device, area.right - 24, 93);
            MoveToEx(device, 24, 100, nullptr); LineTo(device, area.right - 24, 100);
            SelectObject(device, oldPen); DeleteObject(pen);
            SetBkMode(device, TRANSPARENT);
            SetTextColor(device, RGB(223, 242, 255));
            HGDIOBJ oldFont = SelectObject(device, titleFont);
            RECT titleArea{24, 37, area.right - 24, 86};
            DrawTextW(device, L"SIGNAL FROM THE SUNKEN STAR", -1, &titleArea,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(device, oldFont);
            EndPaint(window, &paint);
            return 0;
        }
        case WM_COMMAND:
            if (uiMode == UiMode::Choices && HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) >= 100) {
                chosenAnswer = LOWORD(wParam) - 99;
                clearInteractiveControls();
                return 0;
            }
            if (uiMode == UiMode::Name && HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == 2) {
                char name[100]{}; GetWindowTextA(nameField, name, 100); enteredName = name;
                clearInteractiveControls();
                return 0;
            }
            break;
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
            SetTextColor(reinterpret_cast<HDC>(wParam), RGB(170, 220, 255));
            SetBkColor(reinterpret_cast<HDC>(wParam), RGB(10, 16, 31));
            return reinterpret_cast<LRESULT>(backgroundBrush);
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

int showChoices(const string& question, const vector<string>& choices) {
    uiMode = UiMode::Choices;
    chosenAnswer = 1;
    HWND prompt = CreateWindowA("STATIC", question.c_str(), WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, gameWindow, nullptr, GetModuleHandle(nullptr), nullptr);
    uiControls.push_back(prompt);
    SendMessageW(prompt, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    for (size_t i = 0; i < choices.size(); ++i) {
        HWND button = CreateWindowA("BUTTON", choices[i].c_str(), WS_CHILD | WS_VISIBLE | BS_MULTILINE,
            0, 0, 0, 0, gameWindow, reinterpret_cast<HMENU>(100 + i), GetModuleHandle(nullptr), nullptr);
        uiControls.push_back(button);
        SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    }
    layoutGameWindow();
    MSG message;
    while (uiMode == UiMode::Choices && GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message); DispatchMessageA(&message);
    }
    return chosenAnswer;
}

string askName() {
    uiMode = UiMode::Name;
    enteredName.clear();
    HWND prompt = CreateWindowA("STATIC", "What is your name, operative?", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, gameWindow, nullptr, GetModuleHandle(nullptr), nullptr);
    nameField = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 0, 0, gameWindow, reinterpret_cast<HMENU>(1), GetModuleHandle(nullptr), nullptr);
    HWND begin = CreateWindowA("BUTTON", "Begin Mission", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, gameWindow, reinterpret_cast<HMENU>(2), GetModuleHandle(nullptr), nullptr);
    uiControls = {prompt, begin};
    for (HWND control : uiControls) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    SendMessageW(nameField, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    layoutGameWindow(); SetFocus(nameField);
    MSG message;
    while (uiMode == UiMode::Name && GetMessageA(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message); DispatchMessageA(&message);
    }
    return enteredName.empty() ? "Operative" : enteredName;
}

void openGameWindow() {
    FreeConsole();
    wchar_t executablePath[MAX_PATH];
    GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    std::wstring fontPath(executablePath);
    fontPath = fontPath.substr(0, fontPath.find_last_of(L"\\/")) + L"\\Orbitron-Regular.ttf";
    AddFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr);
    orbitron = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Orbitron");
    titleFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Orbitron");

    WNDCLASSA windowClass{};
    windowClass.lpfnWndProc = GameWindowProc;
    windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = backgroundBrush;
    windowClass.lpszClassName = "CYOAGameWindow";
    RegisterClassA(&windowClass);
    gameWindow = CreateWindowExA(0, "CYOAGameWindow", "Signal from the Sunken Star",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 900, 760,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    std::cout.rdbuf(&storyBuffer);
}
#endif

enum class Calling { Explorer, Pilot, Voyager, Scholar };

class Hero {
public:
    string name;
    Calling calling;
    int health = 3;
    int resolve = 2;
    int clues = 0;
    bool hasRelic = false;
    bool hasAlly = false;
    bool knowsPassword = false;
};

void clearScreen() {
#ifdef _WIN32
    storyText.clear();
    refreshStory();
#else
    cout << "\x1B[2J\x1B[H";
#endif
}

void line() {
#ifndef _WIN32
    cout << "==============================================================\n";
#endif
}

void pause() {
#ifdef _WIN32
    showChoices("Continue your journey when you are ready.", {"Continue"});
#else
    cout << "\nPress Enter to continue...";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif
}

int choose(const string& question, const vector<string>& choices) {
#ifdef _WIN32
    return showChoices(question, choices);
#else
    while (true) {
        cout << "\n" << question << "\n";
        for (size_t i = 0; i < choices.size(); ++i)
            cout << "  " << i + 1 << ") " << choices[i] << "\n";
        cout << "> ";
        int answer;
        if (cin >> answer && answer >= 1 && answer <= static_cast<int>(choices.size())) {
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return answer;
        }
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Please enter one of the numbered choices.\n";
    }
#endif
}

string callingName(Calling calling) {
    switch (calling) {
        case Calling::Explorer: return "Explorer";
        case Calling::Pilot: return "Pilot";
        case Calling::Voyager: return "Voyager";
        default: return "Scholar";
    }
}

void showStatus(const Hero& hero) {
    line();
    cout << hero.name << " the " << callingName(hero.calling)
         << "  |  Health: " << hero.health
         << "  Resolve: " << hero.resolve
         << "  Clues: " << hero.clues
         << "  Device: " << (hero.hasRelic ? "yes" : "no") << "\n";
    line();
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
    int path = choose("What does an Explorer do?", {"Follow the drone's hidden survey route.", "Force open the research hatch.", "Climb a canopy tower to scan the valley."});
    if (path == 1) {
        hero.hasAlly = true; hero.clues++;
        cout << "The drone leads you to a field scientist's shelter and uploads a star map marker for Aramore Station. The scientist joins your mission.\n";
    } else if (path == 2) {
        hero.health--;
        cout << "The hatch opens onto defensive bio-vines. You escape with a few cuts, but recover an access chip marked with the relay's symbol.\n";
        hero.clues++;
    } else {
        hero.resolve++;
        cout << "From the tower, you spot a debris field above Aramore Station and plot a safer approach vector. Your confidence hardens.\n";
    }
    pause();
}

void cityStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "ARAMORE ORBITAL STATION\n\nYou arrive during a cargo transfer. The docking ring is sealing, and an anxious systems engineer presses an access key into your palm. Security drones are scanning every passenger.\n";
    int path = choose("What does a Pilot do?", {"Talk your way through the docking inspection.", "Hide inside a freight container.", "Follow the engineer into the service level."});
    if (path == 1) {
        hero.resolve++;
        cout << "A calm explanation and a well-timed rumor convince the docking chief to clear you. She quietly tells you that the command deck was abandoned.\n";
        hero.clues++;
    } else if (path == 2) {
        hero.health--;
        cout << "The coolant fumes make you sneeze at the worst possible moment. You escape a short chase, carrying a note that reads: 'Below the comms array.'\n";
        hero.clues++;
    } else {
        hero.hasAlly = true;
        cout << "The engineer reveals herself as a resistance courier. She gives you the access key and promises to meet you beneath the comms array.\n";
    }
    pause();
}

void coastStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE SALTGLASS NEBULA\n\nA derelict freighter emerges from a mirror-bright dust cloud. Its emergency beacon repeats inside the hull while a salvage pilot begs you not to answer it. An ion storm is closing in quickly.\n";
    int path = choose("What does a Voyager do?", {"Board the freighter and disable the beacon.", "Question the frightened salvage pilot.", "Use a sensor sweep to map the debris field."});
    if (path == 1) {
        hero.hasRelic = true;
        cout << "Inside, you find a quantum compass that points toward the relay signal.\n";
        cout << "The beacon goes quiet, and the ion storm begins to disperse.\n";
    } else if (path == 2) {
        hero.clues += 2;
        cout << "The pilot admits that he carried a masked stranger to the relay access point. The stranger's password was: 'The sky remembers.'\n";
        hero.knowsPassword = true;
    } else {
        hero.resolve++;
        cout << "Your sweep reveals an ancient jump corridor through the debris. You take a shard of encoded star-metal as an electromagnetic shield.\n";
    }
    pause();
}

void ruinsStart(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE OBSIDIAN DATA ARCHIVE\n\nDust drifts through servers older than the colony. A luminous data core cycles through its own files while a sealed observatory tracks a point beyond the moonless horizon.\n";
    int path = choose("What does a Scholar do?", {"Read the luminous data core.", "Align the observatory array.", "Search the restricted servers for a weapon."});
    if (path == 1) {
        hero.knowsPassword = true; hero.clues++;
        cout << "The data core reveals the relay's original command: 'The sky remembers.'\n";
        cout << "It warns that force strengthens the hostile intelligence below.\n";
    } else if (path == 2) {
        hero.hasRelic = true;
        cout << "A laser alignment opens a concealed compartment containing a prism scanner.\n";
        cout << "It can reveal the source code behind any projected illusion.\n";
    } else {
        hero.health--;
        cout << "A security hologram objects to your methods. You defeat it and recover its plasma cutter, but not without a painful burn.\n";
        hero.resolve++;
    }
    pause();
}

bool bellTower(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE COMMS ARRAY\n\nAll routes lead to Aramore's silent communications array. Its access panel shows an eye, a hand, and a wave. A masked security android steps from the shadows: 'Name the memory that outlives the stars.'\n";
    int action = choose("Choose your response.", {"Say the relay password.", "Offer a device and request access.", "Disable the android.", "Study the interface for another route."});
    if (action == 1) {
        if (hero.knowsPassword) {
            cout << "'The sky remembers.' The android stands aside and opens the access hatch.\n";
            hero.clues++;
        } else {
            hero.health--;
            cout << "The android's mask tilts. 'A memory cannot be guessed.' Its shock baton sends you tumbling down the corridor, but the hatch opens after you.\n";
        }
    } else if (action == 2) {
        if (hero.hasRelic) {
            cout << "The android accepts your device, then returns it recalibrated: it now glows near the relay. You are granted access.\n";
            hero.resolve++;
        } else {
            hero.health--;
            cout << "You have nothing useful to offer. The android rejects you with a concussive blast that nevertheless reveals the maintenance shaft behind it.\n";
        }
    } else if (action == 3) {
        hero.health--;
        cout << "The android is fast, but you disable its control mask. Beneath it is an autonomous service unit, suddenly free and ashamed. It opens the way.\n";
        hero.hasAlly = true;
    } else {
        if (hero.clues >= 2) {
            cout << "The symbols reveal a route through an old maintenance conduit. You bypass the android and find the lift descending to the relay.\n";
        } else {
            hero.health--;
            cout << "The false door drops you into a coolant channel. You crawl out at the correct lift, weaker but wiser.\n";
        }
    }
    pause();
    return hero.health > 0;
}

void finale(Hero& hero) {
    clearScreen(); showStatus(hero);
    cout << "THE SUNKEN STAR RELAY\n\nFar below the station, a blue quantum core hangs above a bottomless reactor shaft. Director Veyr stands before it, wearing a black-glass neural crown. 'One perfect route,' he says, 'and no ship will ever be lost again.'\n\nThe core pulses. Every future you might choose flickers in its light.\n";
    int ending = choose("What will you do?", {"Destroy the neural crown.", "Reason with Veyr and share the burden.", "Use the relay to create one perfect system.", "Shut down the relay forever."});
    clearScreen(); line();
    if (ending == 1) {
        if (hero.resolve >= 3 || hero.hasAlly) {
            cout << "THE OPEN SKIES ENDING\n\nWith your ally holding the signal firewall, you shatter the crown. Veyr collapses, weeping, and the core becomes a thousand harmless lights. The star lanes return: messy, dangerous, and gloriously free.\n";
        } else {
            cout << "THE LAST TRANSMISSION ENDING\n\nThe crown breaks, but its fragments overload your suit. You save the sector as the relay's final transmission carries your name into legend.\n";
        }
    } else if (ending == 2) {
        if (hero.clues >= 2 || hero.knowsPassword) {
            cout << "THE SIGNAL KEEPER ENDING\n\nYou speak the relay command and tell Veyr what its original engineers learned: no algorithm can replace a life. He yields. Together, you turn the relay into a beacon for travelers, not a chain.\n";
        } else {
            cout << "THE UNFINISHED PROTOCOL ENDING\n\nVeyr hesitates, but fear wins. You escape the collapsing chamber with a warning: kindness needs knowledge as well as courage.\n";
        }
    } else if (ending == 3) {
        cout << "THE GOLDEN CAGE ENDING\n\nFor one shining year, no pilot is lost and no choice hurts. Then the people of Aramore begin dreaming of jump gates they cannot open. You govern a perfect system and wonder whether it is still alive.\n";
    } else {
        cout << "THE QUIET ORBIT ENDING\n\nYou shut down the relay with every unit of power you can spare. The sector is safe, though its old wonders fade. Years later, children still tell stories about the " << callingName(hero.calling) << " who chose a quiet orbit.\n";
    }
    line();
}

int main() {
#ifdef _WIN32
    openGameWindow();
#endif
    Hero hero;
    clearScreen();
#ifdef _WIN32
    hero.name = askName();
#else
    cout << "What is your name, operative? ";
    std::getline(cin, hero.name);
#endif
    if (hero.name.empty()) hero.name = "Operative";
    int selected = choose("Choose your calling. Your calling changes where your story begins.", {
        "Explorer - begins in the Verdant-9 biosphere.",
        "Pilot - begins on Aramore Orbital Station.",
        "Voyager - begins in the Saltglass Nebula.",
        "Scholar - begins in the Obsidian Data Archive."});
    hero.calling = static_cast<Calling>(selected - 1);
    opening(hero);
    switch (hero.calling) {
        case Calling::Explorer: forestStart(hero); break;
        case Calling::Pilot: cityStart(hero); break;
        case Calling::Voyager: coastStart(hero); break;
        case Calling::Scholar: ruinsStart(hero); break;
    }
    if (hero.health > 0 && bellTower(hero)) finale(hero);
    else {
        clearScreen(); line();
        cout << "THE MISSION PAUSES\n\nYou awaken in an Aramore Station med bay, alive but\n"
                "changed. The relay still broadcasts beneath the station. Another mission awaits.\n";
        line();
    }
    cout << "\nThank you for playing.";
#ifdef _WIN32
    showChoices("Your mission has ended.", {"Close Game"});
#else
    cout << " Press Enter to close the mission.";
    cin.get();
#endif
    return 0;
}
