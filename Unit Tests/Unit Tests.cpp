#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../File Explorer/TheHeader.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
    TEST_CLASS(StringHelpers)
    {
    public:
        TEST_METHOD(ToLower_ConvertsUppercaseAscii)
        {
            Assert::AreEqual("hello world", toLower("HELLO WORLD").c_str());
        }

        TEST_METHOD(ToLower_LeavesAlreadyLowercaseUnchanged)
        {
            Assert::AreEqual("already", toLower("already").c_str());
        }

        TEST_METHOD(ToLower_HandlesEmptyString)
        {
            Assert::AreEqual("", toLower("").c_str());
        }

        TEST_METHOD(Fit_PadsShortStringToWidth)
        {
            Assert::AreEqual("ab   ", fit("ab", 5).c_str());
        }

        TEST_METHOD(Fit_TruncatesLongStringWithTilde)
        {
            Assert::AreEqual("abc~", fit("abcdef", 4).c_str());
        }

        TEST_METHOD(Fit_ReturnsExactLengthStringUnchanged)
        {
            Assert::AreEqual("abcde", fit("abcde", 5).c_str());
        }

        TEST_METHOD(Fit_ZeroWidthReturnsEmpty)
        {
            Assert::AreEqual("", fit("abc", 0).c_str());
        }
    };

    TEST_CLASS(ExtensionHelpers)
    {
    public:
        TEST_METHOD(GetExt_ReturnsLowercaseExtension)
        {
            Assert::AreEqual("cpp", getExt(fs::path("Main.CPP")).c_str());
        }

        TEST_METHOD(GetExt_NoExtensionReturnsEmpty)
        {
            Assert::AreEqual("", getExt(fs::path("README")).c_str());
        }

        TEST_METHOD(GetExt_DotfileReturnsEmpty)
        {

            Assert::AreEqual("", getExt(fs::path(".gitignore")).c_str());
        }

        TEST_METHOD(GetExt_HandlesUnicodeStem)
        {
            Assert::AreEqual("txt", getExt(fs::path(L"Dokumentć.txt")).c_str());
        }
    };

    TEST_CLASS(PathConversion)
    {
    public:
        TEST_METHOD(PathToUtf8_RoundTripsAsciiPath)
        {
            Assert::AreEqual("plain.txt", pathToUtf8(fs::path(L"plain.txt")).c_str());
        }

        TEST_METHOD(PathToUtf8_DoesNotThrowOnNonAsciiCharacters)
        {
            // valid UTF-8 .
            fs::path p(L"Folderć"); 
            std::string utf8;
            bool threw = false;
            try {
                utf8 = pathToUtf8(p);
            }
            catch (...) {
                threw = true;
            }
            Assert::IsFalse(threw);
            Assert::IsTrue(utf8.find("Folder") == 0);
        }

        TEST_METHOD(PathToUtf8_EmptyPathReturnsEmptyString)
        {
            Assert::AreEqual("", pathToUtf8(fs::path()).c_str());
        }
    };

    TEST_CLASS(SizeFormatting)
    {
    public:
        TEST_METHOD(HumanSize_ZeroReturnsDash)
        {
            Assert::AreEqual("-", humanSize(0).c_str());
        }

        TEST_METHOD(HumanSize_BytesUnderOneKB)
        {
            Assert::AreEqual("512 B", humanSize(512).c_str());
        }

        TEST_METHOD(HumanSize_FormatsExactKilobyte)
        {
            Assert::AreEqual("1.0 KB", humanSize(1024).c_str());
        }

        TEST_METHOD(HumanSize_FormatsMegabytes)
        {
            uintmax_t bytes = static_cast<uintmax_t>(1.5 * 1024 * 1024);
            Assert::AreEqual("1.5 MB", humanSize(bytes).c_str());
        }

        TEST_METHOD(HumanSize_CapsUnitAtGigabytes)
        {
            uintmax_t huge = static_cast<uintmax_t>(5) * 1024 * 1024 * 1024 * 1024; // 5 TB
            std::string result = humanSize(huge);
            Assert::IsTrue(result.find("GB") != std::string::npos);
        }
    };

    TEST_CLASS(EntryStyling)
    {
    public:
        TEST_METHOD(EntryStyle_DirectoryUsesDirIcon)
        {
            Entry e{ "folder", fs::path("folder"), true, 0, "" };
            auto styled = entryStyle(e);
            Assert::AreEqual("[DIR]  ", styled.first.c_str());
        }

        TEST_METHOD(EntryStyle_KnownExtensionUsesMappedIcon)
        {
            Entry e{ "main.cpp", fs::path("main.cpp"), false, 100, "cpp" };
            auto styled = entryStyle(e);
            Assert::AreEqual("[C++] ", styled.first.c_str());
        }

        TEST_METHOD(EntryStyle_UnknownExtensionUsesGenericIcon)
        {
            Entry e{ "data.xyz", fs::path("data.xyz"), false, 100, "xyz" };
            auto styled = entryStyle(e);
            Assert::AreEqual("[FILE] ", styled.first.c_str());
        }
    };

    TEST_CLASS(DirectoryLoading)
    {
    public:
        TEST_METHOD(LoadDir_ListsParentSubdirAndFile)
        {
            fs::path temp = fs::temp_directory_path() / "FileExplorerTests_LoadDir";
            std::error_code ec;
            fs::remove_all(temp, ec);
            fs::create_directories(temp / "sub");
            {
                std::ofstream f((temp / "a.txt").string());
                f << "hello";
            }

            auto entries = loadDir(temp);

            bool hasParent = false, hasSub = false, hasFile = false;
            for (const auto& e : entries) {
                if (e.name == "..") hasParent = true;
                if (e.name == "sub" && e.isDir) hasSub = true;
                if (e.name == "a.txt" && !e.isDir) hasFile = true;
            }

            Assert::IsTrue(hasParent, L"expected a '..' entry");
            Assert::IsTrue(hasSub, L"expected to find subdirectory 'sub'");
            Assert::IsTrue(hasFile, L"expected to find file 'a.txt'");

            fs::remove_all(temp, ec);
        }

        TEST_METHOD(LoadDir_HandlesUnicodeFolderNameWithoutThrowing)
        {
            fs::path temp = fs::temp_directory_path() / L"FileExplorerTests_Prića";
            std::error_code ec;
            fs::remove_all(temp, ec);
            fs::create_directories(temp);

            std::vector<Entry> entries;
            bool threw = false;
            try {
                entries = loadDir(temp);
            }
            catch (...) {
                threw = true;
            }

            Assert::IsFalse(threw);
            Assert::IsTrue(entries.size() >= 1); 

            fs::remove_all(temp, ec);
        }
    };
}
