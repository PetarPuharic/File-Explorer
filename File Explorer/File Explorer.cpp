#include "TheHeader.h"

// Main function
int main(int argc, char* argv[]) {
    enableAnsi();
    fs::path cwd = (argc > 1) ? fs::path(argv[1]) : fs::current_path(); // Start in current dir

    hideCursor();

    auto entries = loadDir(cwd);
    int  sel = 0;
    int  scroll = 0;

    auto clampScroll = [&]() {
        int listH = termHeight() - 5;
        if (sel < scroll)           scroll = sel;
        if (sel >= scroll + listH)  scroll = sel - listH + 1;
        scroll = std::max(0, scroll);
        };

    render(cwd, entries, sel, scroll);
    while (true) {
        Key k = readKey();
        if (k == Key::Quit) break;

        if (k == Key::Up && sel > 0) {
            --sel; clampScroll();
        }
        else if (k == Key::Down && sel < static_cast<int>(entries.size()) - 1) {
            ++sel; clampScroll();
        }
        else if (k == Key::Enter && !entries.empty() && entries[sel].isDir) {
            fs::path next = entries[sel].path;
            std::error_code ec;
            if (fs::is_directory(next, ec)) {
                cwd = next; entries = loadDir(cwd); sel = 0; scroll = 0;
            }
        }

        render(cwd, entries, sel, scroll);
    }
    showCursor();
    clearScreen();
    std::cout << "Bye!\n";
    return 0;
}