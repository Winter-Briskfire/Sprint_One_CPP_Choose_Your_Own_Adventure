# Signal from the Sunken Star

A science-fiction choose-your-own-adventure game written in C++ for CSE 310.

## Build and run

On Windows with MSYS2 MinGW, run:

```powershell
g++ -std=c++17 -Wall -Wextra -mwindows CYOA_main.cpp -o CYOA_game.exe -lgdi32
.\CYOA_game.exe
```

Keep `Orbitron-Regular.ttf` in the same folder as the executable so the game can load its custom font.

## C++ concepts demonstrated

- Conditionals for branching choices and endings
- Loops for GUI controls and input handling
- Functions for game scenes and interface behavior
- Classes, including `Hero` and `StoryBuffer`
- STL `vector` containers for choices and GUI controls
