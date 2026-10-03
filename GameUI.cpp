#include "GameUI.hpp"

#include <iostream>
#include <limits>
#include <streambuf>

#ifdef _WIN32
#include <windows.h>

namespace {
HWND gameWindow = nullptr;
HWND storyBox = nullptr;
HFONT orbitron = nullptr;
HFONT titleFont = nullptr;
HBRUSH backgroundBrush = CreateSolidBrush(RGB(10, 16, 31));
std::string storyText;

enum class UiMode { None, Name, Choices };
UiMode uiMode = UiMode::None;
std::vector<HWND> uiControls;
HWND nameField = nullptr;
std::string enteredName;
int chosenAnswer = 1;

std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) return L"";
    int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, result.data(), length);
    result.pop_back();
    return result;
}

std::string effectivenessPrefix(ChoiceEffectiveness effectiveness) {
    switch (effectiveness) {
        case ChoiceEffectiveness::High: return u8"\u2191 ";
        case ChoiceEffectiveness::Low: return u8"\u2193 ";
        default: return u8"\u2212 ";
    }
}

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
            if (character == '\n' && (!storyText.empty() && storyText.back() != '\r')) storyText.push_back('\r');
            storyText.push_back(static_cast<char>(character));
            refreshStory();
        }
        return character;
    }

    std::streamsize xsputn(const char* text, std::streamsize count) override {
        for (std::streamsize i = 0; i < count; ++i) overflow(text[i]);
        return count;
    }
};

StoryBuffer storyBuffer;

void layoutGameWindow() {
    if (!gameWindow || !storyBox) return;
    RECT area{}; GetClientRect(gameWindow, &area);
    int width = area.right;
    int height = area.bottom;
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
            storyBox = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                0, 0, 0, 0, window, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessageW(storyBox, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
            return 0;
        case WM_SIZE: layoutGameWindow(); return 0;
        case WM_PAINT: {
            PAINTSTRUCT paint{}; HDC device = BeginPaint(window, &paint); RECT area{}; GetClientRect(window, &area);
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(97, 204, 255)); HGDIOBJ oldPen = SelectObject(device, pen);
            MoveToEx(device, 24, 22, nullptr); LineTo(device, area.right - 24, 22);
            MoveToEx(device, 24, 29, nullptr); LineTo(device, area.right - 24, 29);
            MoveToEx(device, 24, 93, nullptr); LineTo(device, area.right - 24, 93);
            MoveToEx(device, 24, 100, nullptr); LineTo(device, area.right - 24, 100);
            SelectObject(device, oldPen); DeleteObject(pen);
            SetBkMode(device, TRANSPARENT); SetTextColor(device, RGB(223, 242, 255));
            HGDIOBJ oldFont = SelectObject(device, titleFont); RECT titleArea{24, 37, area.right - 24, 86};
            DrawTextW(device, L"SIGNAL FROM THE SUNKEN STAR", -1, &titleArea, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(device, oldFont); EndPaint(window, &paint); return 0;
        }
        case WM_COMMAND:
            if (uiMode == UiMode::Choices && HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) >= 100) {
                chosenAnswer = LOWORD(wParam) - 99; clearInteractiveControls(); return 0;
            }
            if (uiMode == UiMode::Name && HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == 2) {
                char name[100]{}; GetWindowTextA(nameField, name, 100); enteredName = name; clearInteractiveControls(); return 0;
            }
            break;
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
            SetTextColor(reinterpret_cast<HDC>(wParam), RGB(170, 220, 255));
            SetBkColor(reinterpret_cast<HDC>(wParam), RGB(10, 16, 31));
            return reinterpret_cast<LRESULT>(backgroundBrush);
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            // The main game window is the application's lifetime. Closing it
            // must end every active input loop and release the executable.
            ExitProcess(0);
    }
    return DefWindowProcA(window, message, wParam, lParam);
}
} // namespace

void openGameWindow() {
    FreeConsole();
    wchar_t executablePath[MAX_PATH]; GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    std::wstring fontPath(executablePath);
    fontPath = fontPath.substr(0, fontPath.find_last_of(L"\\/")) + L"\\Orbitron-Regular.ttf";
    AddFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr);
    orbitron = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Orbitron");
    titleFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Orbitron");
    WNDCLASSA windowClass{};
    windowClass.lpfnWndProc = GameWindowProc; windowClass.hInstance = GetModuleHandle(nullptr);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW); windowClass.hbrBackground = backgroundBrush;
    windowClass.lpszClassName = "CYOAGameWindow"; RegisterClassA(&windowClass);
    gameWindow = CreateWindowExA(0, "CYOAGameWindow", "Signal from the Sunken Star", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 760, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    std::cout.rdbuf(&storyBuffer);
}

void clearScreen() { storyText.clear(); refreshStory(); }
void line() {}

int choose(const std::string& question, const std::vector<std::string>& choices,
           const std::vector<ChoiceEffectiveness>& effectiveness, bool showIndicators) {
    uiMode = UiMode::Choices; chosenAnswer = 1;
    HWND prompt = CreateWindowA("STATIC", question.c_str(), WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, gameWindow, nullptr, GetModuleHandle(nullptr), nullptr);
    uiControls.push_back(prompt); SendMessageW(prompt, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    for (size_t i = 0; i < choices.size(); ++i) {
        ChoiceEffectiveness rating = i < effectiveness.size() ? effectiveness[i] : ChoiceEffectiveness::Neutral;
        std::wstring label = utf8ToWide((showIndicators ? effectivenessPrefix(rating) : "") + choices[i]);
        HWND button = CreateWindowW(L"BUTTON", label.c_str(), WS_CHILD | WS_VISIBLE | BS_MULTILINE, 0, 0, 0, 0,
            gameWindow, reinterpret_cast<HMENU>(100 + i), GetModuleHandle(nullptr), nullptr);
        uiControls.push_back(button); SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    }
    layoutGameWindow(); MSG message;
    while (uiMode == UiMode::Choices && GetMessageA(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageA(&message); }
    return chosenAnswer;
}

std::string askName() {
    uiMode = UiMode::Name; enteredName.clear();
    HWND prompt = CreateWindowA("STATIC", "What is your name, operative?", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, gameWindow, nullptr, GetModuleHandle(nullptr), nullptr);
    nameField = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, gameWindow, reinterpret_cast<HMENU>(1), GetModuleHandle(nullptr), nullptr);
    HWND begin = CreateWindowA("BUTTON", "Begin Mission", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, gameWindow, reinterpret_cast<HMENU>(2), GetModuleHandle(nullptr), nullptr);
    uiControls = {prompt, begin};
    for (HWND control : uiControls) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE);
    SendMessageW(nameField, WM_SETFONT, reinterpret_cast<WPARAM>(orbitron), TRUE); layoutGameWindow(); SetFocus(nameField);
    MSG message;
    while (uiMode == UiMode::Name && GetMessageA(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageA(&message); }
    return enteredName.empty() ? "Operative" : enteredName;
}

void pause() {
    static bool introductionAcknowledged = false;
    if (introductionAcknowledged) return;
    introductionAcknowledged = true;
    choose("Continue your mission when you are ready.", {"Continue"}, {}, false);
}

#else

void openGameWindow() {}
void clearScreen() { std::cout << "\x1B[2J\x1B[H"; }
void line() { std::cout << "==============================================================\n"; }
void pause() {
    static bool introductionAcknowledged = false;
    if (introductionAcknowledged) return;
    introductionAcknowledged = true;
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
int choose(const std::string& question, const std::vector<std::string>& choices,
           const std::vector<ChoiceEffectiveness>& effectiveness, bool showIndicators) {
    while (true) {
        std::cout << "\n" << question << "\n";
        for (size_t i = 0; i < choices.size(); ++i) {
            ChoiceEffectiveness rating = i < effectiveness.size() ? effectiveness[i] : ChoiceEffectiveness::Neutral;
            const char* marker = rating == ChoiceEffectiveness::High ? "[UP]" : rating == ChoiceEffectiveness::Low ? "[DOWN]" : "[-]";
            std::cout << "  " << i + 1 << ") " << (showIndicators ? marker : "")
                      << (showIndicators ? " " : "") << choices[i] << "\n";
        }
        std::cout << "> "; int answer;
        if (std::cin >> answer && answer >= 1 && answer <= static_cast<int>(choices.size())) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return answer;
        }
        std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Please enter one of the numbered choices.\n";
    }
}
std::string askName() { std::string name; std::cout << "What is your name, operative? "; std::getline(std::cin, name); return name.empty() ? "Operative" : name; }
#endif
