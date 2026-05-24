#ifndef HEADER_INCLUDED_
#define HEADER_INCLUDED_
#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <conio.h>
#include <windows.h>
#include <unordered_map>
#include <iostream>

// Define shorthand for codes
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

namespace fs = std::filesystem;

struct Entry {
    std::string name;
    bool        isDir;
    uintmax_t   size;
    std::string ext;
};
enum class Key { Up, Down, Enter, Quit, Other };

void showCursor();
void hideCursor();
void clearScreen();

int termWidth();
int termHeight();
void enableAnsi();
std::string toLower(std::string s);
std::string humanSize(uintmax_t bytes);
std::string fit(const std::string& s, int w);

std::string getExt(const std::string& name);
std::pair<std::string, std::string> entryStyle(const Entry& e);
std::vector<Entry> loadDir(const fs::path& dir);

void render(const fs::path& cwd, const std::vector<Entry>& entries, int sel, int scroll);

Key readKey();

#endif