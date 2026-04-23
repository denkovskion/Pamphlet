/*
 * MIT License
 *
 * Copyright (c) 2026 Ivan Denkovski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <exception>
#include <iostream>
#include <regex>

#include "Problem.h"
#include "Version.h"

#if defined(_WIN64)
static const char* const PLATFORM_SFX = " (x64)";
#elif defined(_WIN32)
static const char* const PLATFORM_SFX = " (x86)";
#else
static const char* const PLATFORM_SFX = "";
#endif

#if defined(_DEBUG) || !defined(NDEBUG)
static const char* const CONFIGURATION_SFX = " [Debug]";
#else
static const char* const CONFIGURATION_SFX = "";
#endif

int main(int argc, char* argv[]) {
  try {
    bool help = false;
    bool version = false;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "--help") {
        help = true;
      } else if (arg == "--version") {
        version = true;
      } else if (std::regex_match(arg, std::regex("-[hV]+"))) {
        for (char letter : arg.substr(1)) {
          if (letter == 'h') {
            help = true;
          } else if (letter == 'V') {
            version = true;
          }
        }
      } else {
        pamphlet::logger(std::clog) << "Invalid argument: '" + arg + "'.\n";
        return 1;
      }
    }
    if (help) {
      std::cout << R"(Usage:
  Pamphlet [OPTIONS]

Chess mate searcher. Reads problems as EPD records (with one operation:
  dm for direct mate or acd for perft) until EOF, then solves them.

Options:
  -h, --help       Show help and exit
  -V, --version    Show version and exit
)";
      return 0;
    }
    if (version) {
      std::cout << std::string() + "Pamphlet " + VERSION_STR + PLATFORM_SFX +
                       CONFIGURATION_SFX + "\n" + "Built on: " + TIMESTAMP_STR +
                       "\n" + "Copyright (c) 2026 Ivan Denkovski\n" +
                       "License: MIT\n";
      return 0;
    }
    pamphlet::logger(std::clog) << std::string() + "Pamphlet " + VERSION_STR +
                                       PLATFORM_SFX + CONFIGURATION_SFX +
                                       " Copyright (c) 2026 Ivan Denkovski\n";
    std::vector<pamphlet::Problem> problems = pamphlet::readAllProblems();
    for (const pamphlet::Problem& problem : problems) {
      pamphlet::write(problem);
      pamphlet::solve(problem);
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
  } catch (...) {
    std::cerr << "Exception\n";
  }
}
