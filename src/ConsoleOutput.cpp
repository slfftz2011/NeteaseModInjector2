#include "ConsoleOutput.h"

#include <iostream>
#include <streambuf>
#include <string>
#include <vector>
#include <windows.h>

namespace {
class Utf8ConsoleBuffer : public std::streambuf {
public:
    explicit Utf8ConsoleBuffer(HANDLE outputHandle) : outputHandle_(outputHandle) {
        DWORD mode = 0;
        isConsole_ = GetConsoleMode(outputHandle_, &mode) != 0;
    }

protected:
    int_type overflow(int_type character) override {
        if (traits_type::eq_int_type(character, traits_type::eof())) {
            return sync() == 0 ? traits_type::not_eof(character) : traits_type::eof();
        }
        pending_.push_back(traits_type::to_char_type(character));
        writeCompleteUtf8(false);
        return character;
    }

    std::streamsize xsputn(const char* data, std::streamsize size) override {
        pending_.append(data, static_cast<size_t>(size));
        writeCompleteUtf8(false);
        return size;
    }

    int sync() override {
        writeCompleteUtf8(true);
        return 0;
    }

private:
    HANDLE outputHandle_;
    bool isConsole_ = false;
    std::string pending_;

    size_t completeUtf8Prefix() const {
        size_t offset = 0;
        while (offset < pending_.size()) {
            const unsigned char lead = static_cast<unsigned char>(pending_[offset]);
            size_t sequenceLength = 1;
            if (lead >= 0xC2 && lead <= 0xDF) {
                sequenceLength = 2;
            } else if (lead >= 0xE0 && lead <= 0xEF) {
                sequenceLength = 3;
            } else if (lead >= 0xF0 && lead <= 0xF4) {
                sequenceLength = 4;
            }

            if (offset + sequenceLength > pending_.size()) {
                break;
            }
            bool validSequence = true;
            for (size_t index = 1; index < sequenceLength; ++index) {
                const unsigned char continuation = static_cast<unsigned char>(pending_[offset + index]);
                if ((continuation & 0xC0) != 0x80) {
                    validSequence = false;
                    break;
                }
            }
            offset += validSequence ? sequenceLength : 1;
        }
        return offset;
    }

    void writeCompleteUtf8(bool flushRemainder) {
        const size_t byteCount = flushRemainder ? pending_.size() : completeUtf8Prefix();
        if (byteCount == 0) {
            return;
        }

        if (!isConsole_) {
            DWORD written = 0;
            WriteFile(outputHandle_, pending_.data(), static_cast<DWORD>(byteCount), &written, nullptr);
        } else {
            const int inputLength = static_cast<int>(byteCount);
            const int wideLength = MultiByteToWideChar(CP_UTF8, 0, pending_.data(), inputLength, nullptr, 0);
            if (wideLength > 0) {
                std::vector<wchar_t> wideText(static_cast<size_t>(wideLength));
                MultiByteToWideChar(CP_UTF8, 0, pending_.data(), inputLength, wideText.data(), wideLength);
                DWORD written = 0;
                WriteConsoleW(outputHandle_, wideText.data(), static_cast<DWORD>(wideLength), &written, nullptr);
            }
        }
        pending_.erase(0, byteCount);
    }
};
}

void initializeConsoleOutput() {
    static Utf8ConsoleBuffer stdoutBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
    static Utf8ConsoleBuffer stderrBuffer(GetStdHandle(STD_ERROR_HANDLE));
    std::cout.rdbuf(&stdoutBuffer);
    std::cerr.rdbuf(&stderrBuffer);
}