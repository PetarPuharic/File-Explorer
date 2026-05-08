#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <conio.h>
#include <windows.h>
#include <unordered_map>

namespace fs = std::filesystem;


#define RESET      "\033[0m"
#define BOLD       "\033[1m"
#define DIM        "\033[2m"
#define BG_BLUE    "\033[44m"
#define BG_ALT     "\033[48;5;235m"
#define FG_WHITE   "\033[97m"
#define FG_YELLOW  "\033[93m"
#define FG_CYAN    "\033[96m"
#define FG_BLUE    "\033[94m"
#define FG_GREEN   "\033[92m"
#define FG_GRAY    "\033[90m"
#define FG_RED     "\033[91m"
#define FG_MAGENTA "\033[95m"


void clearScreen() { system("cls"); }
void hideCursor() { std::cout << "\033[?25l"; }
void showCursor() { std::cout << "\033[?25h"; }

int termWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int termHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void enableAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

struct Entry {
    std::string name;
    bool        isDir;
    uintmax_t   size;
    std::string ext;
};

std::string toLower(std::string s) { //SelfExplanatory
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

std::string getExt(const std::string& name) {
    auto dot = name.rfind('.');
    if (dot == std::string::npos || dot == 0) return "";
    return toLower(name.substr(dot + 1));
}

std::pair<std::string, std::string> entryStyle(const Entry& e) {
    if (e.isDir) return { "[DIR]  ", FG_YELLOW };

    static const std::unordered_map<std::string, std::pair<std::string, std::string>> MAP = {
        {"cpp",  {"[C++] ", FG_BLUE}},  {"c",    {"[C]   ", FG_BLUE}},
        {"h",    {"[H]   ", FG_CYAN}},  {"hpp",  {"[H]   ", FG_CYAN}},
        {"py",   {"[PY]  ", FG_GREEN}}, {"js",   {"[JS]  ", FG_YELLOW}},
        {"ts",   {"[TS]  ", FG_BLUE}},  {"rs",   {"[RS]  ", FG_RED}},
        {"go",   {"[GO]  ", FG_CYAN}},  {"cs",   {"[CS]  ", FG_MAGENTA}},
        {"sh",   {"[SH]  ", FG_GREEN}}, {"bat",  {"[BAT] ", FG_GREEN}},
        {"md",   {"[MD]  ", FG_WHITE}}, {"txt",  {"[TXT] ", FG_WHITE}},
        {"json", {"[JSON]", FG_YELLOW}},{"yaml", {"[YAML]", FG_YELLOW}},
        {"xml",  {"[XML] ", FG_YELLOW}},{"html", {"[HTML]", FG_RED}},
        {"css",  {"[CSS] ", FG_BLUE}},  {"png",  {"[IMG] ", FG_MAGENTA}},
        {"jpg",  {"[IMG] ", FG_MAGENTA}},{"jpeg",{"[IMG] ", FG_MAGENTA}},
        {"gif",  {"[IMG] ", FG_MAGENTA}},{"svg", {"[SVG] ", FG_MAGENTA}},
        {"pdf",  {"[PDF] ", FG_RED}},   {"zip",  {"[ZIP] ", FG_YELLOW}},
        {"exe",  {"[EXE] ", FG_GREEN}}, {"dll",  {"[DLL] ", FG_RED}},
        {"mp3",  {"[AUD] ", FG_GREEN}}, {"mp4",  {"[VID] ", FG_MAGENTA}},
    };

    auto it = MAP.find(e.ext);
    if (it != MAP.end()) return it->second;
    return { "[FILE] ", FG_WHITE };
}

std::string humanSize(uintmax_t bytes) {
    if (bytes == 0) return "-";
    const char* units[] = { "B", "KB", "MB", "GB" };
    int u = 0; double v = (double)bytes;
    while (v >= 1024.0 && u < 3) { v /= 1024.0; ++u; }
    char buf[20];
    if (u == 0) snprintf(buf, sizeof(buf), "%llu B", (unsigned long long)bytes);
    else        snprintf(buf, sizeof(buf), "%.1f %s", v, units[u]);
    return buf;
}

std::string fit(const std::string& s, int w) {
    if (w <= 0) return "";
    if ((int)s.size() > w) return s.substr(0, w - 1) + "~";
    return s + std::string(w - (int)s.size(), ' ');
}

// LOADING DIRESCTORY ENTRIES
std::vector<Entry> loadDir(const fs::path& dir) {
    std::vector<Entry> entries;
    if (dir.has_parent_path() && dir != dir.root_path())
        entries.push_back({ "..", true, 0, "" });

    try {
        for (const auto& de : fs::directory_iterator(dir,
            fs::directory_options::skip_permission_denied)) {
            Entry e;
            e.name = de.path().filename().string();
            e.isDir = de.is_directory();
            e.ext = getExt(e.name);
            if (!e.isDir) {
                std::error_code ec;
                e.size = fs::file_size(de.path(), ec);
                if (ec) e.size = 0;
            }
            entries.push_back(e);
        }
    }
    catch (...) {}

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.name == "..") return true;
        if (b.name == "..") return false;
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return toLower(a.name) < toLower(b.name);
        });
    return entries;
}

// UI RENDER
void render(const fs::path& cwd, const std::vector<Entry>& entries, int sel, int scroll) {
    int W = termWidth();
    int H = termHeight();
    int listH = H - 5;
    int nameW = W - 22;

    clearScreen();

    // Title bar
    std::string title = " >> " + cwd.string() + " ";
    std::cout << BG_BLUE << BOLD << FG_WHITE << fit(title, W) << RESET << "\n";

    // Column header
    std::string hdr = "  " + fit("  Name", nameW) + fit("Type  ", 10) + fit("Size", 8);
    std::cout << BG_ALT << FG_GRAY << BOLD << fit(hdr, W) << RESET << "\n";
    std::cout << FG_GRAY << DIM << std::string(W, '-') << RESET << "\n";

    // File rows
    int end = min((int)entries.size(), scroll + listH);
    for (int i = scroll; i < end; ++i) {
        const Entry& e = entries[i];
        bool         hi = (i == sel);
        auto [icon, col] = entryStyle(e);

        std::string typeStr = e.isDir ? "dir" : (e.ext.empty() ? "file" : e.ext);
        std::string sizeStr = e.isDir ? "-" : humanSize(e.size);
        std::string row = "  " + icon + fit(e.name, nameW - 8) + "  "
            + fit(typeStr, 8) + fit(sizeStr, 8);

        if (hi) {
            std::cout << BG_BLUE << BOLD << FG_WHITE << fit(row, W) << RESET << "\n";
        }
        else {
            if ((i - scroll) % 2 == 1) std::cout << BG_ALT;
            std::cout << col << fit(row, W) << RESET << "\n";
        }
    }

    // Fill empty rows
    for (int r = (end - scroll); r < listH; ++r)
        std::cout << std::string(W, ' ') << "\n";

    // Status bar
    std::string status = " " + std::to_string(sel + 1) + "/" +
        std::to_string(entries.size()) + "  " +
        (entries.empty() ? "" : entries[sel].name);
    std::string keys = " [Up/Down] navigate  [Enter] open dir  [Q] quit ";
    std::cout << FG_GRAY << DIM << std::string(W, '-') << RESET << "\n";
    std::cout << BG_ALT << FG_CYAN << BOLD << status
        << std::string(max(0, W - (int)status.size() - (int)keys.size()), ' ')
        << FG_GRAY << RESET << BG_ALT << keys << RESET;
    std::cout.flush();
}

// Input handling
enum class Key { Up, Down, Enter, Quit, Other };

Key readKey() {
    int c = _getch();
    if (c == 'q' || c == 'Q') return Key::Quit;
    if (c == '\r')             return Key::Enter;
    if (c == 0 || c == 0xE0) {
        int c2 = _getch();
        if (c2 == 72) return Key::Up;
        if (c2 == 80) return Key::Down;
    }
    return Key::Other;
}

// Main function
int main(int argc, char* argv[]) {
    enableAnsi();
    fs::path cwd = (argc > 1) ? fs::path(argv[1]) : fs::current_path();

    hideCursor();

    auto entries = loadDir(cwd);
    int  sel = 0;
    int  scroll = 0;

    auto clampScroll = [&]() {
        int listH = termHeight() - 5;
        if (sel < scroll)           scroll = sel;
        if (sel >= scroll + listH)  scroll = sel - listH + 1;
        scroll = max(0, scroll);
        };

    render(cwd, entries, sel, scroll);

    while (true) {
        Key k = readKey();
        if (k == Key::Quit) break;

        if (k == Key::Up && sel > 0) {
            --sel; clampScroll();
        }
        else if (k == Key::Down && sel < (int)entries.size() - 1) {
            ++sel; clampScroll();
        }
        else if (k == Key::Enter && !entries.empty() && entries[sel].isDir) {
            fs::path next = (entries[sel].name == "..") ? cwd.parent_path()
                : cwd / entries[sel].name;
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