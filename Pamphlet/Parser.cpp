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

#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include "Problem.h"

namespace pamphlet {

std::vector<Problem> readAllProblems() {
  std::vector<Problem> problems;
  for (std::string line; std::getline(std::cin, line);) {
    std::istringstream tokens(line);
    if (std::string token; tokens >> token) {
      std::map<Square, Piece> board;
      std::istringstream symbols(token);
      int rank = 7;
      for (; rank >= 0; --rank) {
        int file = 0;
        for (; file <= 7; ++file) {
          char symbol = symbols.get();
          if (symbol >= '1' && symbol <= '8' - file) {
            file += symbol - '0';
            if (file > 7) {
              break;
            }
            symbol = symbols.get();
          }
          if (symbol == 'K') {
            board.insert({{file, rank}, {PieceType::KING, false}});
          } else if (symbol == 'Q') {
            board.insert({{file, rank}, {PieceType::QUEEN, false}});
          } else if (symbol == 'R') {
            board.insert({{file, rank}, {PieceType::ROOK, false}});
          } else if (symbol == 'B') {
            board.insert({{file, rank}, {PieceType::BISHOP, false}});
          } else if (symbol == 'N') {
            board.insert({{file, rank}, {PieceType::KNIGHT, false}});
          } else if (symbol == 'P') {
            board.insert({{file, rank}, {PieceType::PAWN, false}});
          } else if (symbol == 'k') {
            board.insert({{file, rank}, {PieceType::KING, true}});
          } else if (symbol == 'q') {
            board.insert({{file, rank}, {PieceType::QUEEN, true}});
          } else if (symbol == 'r') {
            board.insert({{file, rank}, {PieceType::ROOK, true}});
          } else if (symbol == 'b') {
            board.insert({{file, rank}, {PieceType::BISHOP, true}});
          } else if (symbol == 'n') {
            board.insert({{file, rank}, {PieceType::KNIGHT, true}});
          } else if (symbol == 'p') {
            board.insert({{file, rank}, {PieceType::PAWN, true}});
          } else {
            break;
          }
        }
        if (file <= 7) {
          break;
        }
        if (rank > 0) {
          if (symbols.get() != '/') {
            break;
          }
        } else {
          if (symbols.get(); !symbols.eof()) {
            break;
          }
        }
      }
      if (rank < 0) {
        if (std::string token; tokens >> token) {
          if (std::regex_match(token, std::regex("[wb]"))) {
            bool blackToMove = false;
            if (token == "b") {
              blackToMove = true;
            }
            if (std::string token; tokens >> token) {
              if (std::regex_match(token, std::regex("\\bK?Q?k?q?|-"))) {
                std::set<Square> castlingOrigins;
                if (!(token == "-")) {
                  for (char symbol : token) {
                    if (symbol == 'K' || symbol == 'Q') {
                      castlingOrigins.insert({4, 0});
                    } else if (symbol == 'k' || symbol == 'q') {
                      castlingOrigins.insert({4, 7});
                    }
                    if (symbol == 'K') {
                      castlingOrigins.insert({7, 0});
                    } else if (symbol == 'Q') {
                      castlingOrigins.insert({0, 0});
                    } else if (symbol == 'k') {
                      castlingOrigins.insert({7, 7});
                    } else if (symbol == 'q') {
                      castlingOrigins.insert({0, 7});
                    }
                  }
                }
                if (std::string token; tokens >> token) {
                  if (std::regex_match(token, std::regex("[a-h][36]|-"))) {
                    std::optional<Square> enPassantTarget;
                    if (!(token == "-")) {
                      int file = token.at(0) - 'a';
                      int rank = token.at(1) - '1';
                      enPassantTarget = {file, rank};
                    }
                    if (std::string token; tokens >> token) {
                      if (std::regex_match(token, std::regex("acd|dm"))) {
                        try {
                          if (token == "acd") {
                            if (std::string token; tokens >> token) {
                              if (std::regex_match(
                                      token, std::regex("(0|[1-9]\\d*);"))) {
                                int nPlies = std::stoi(token);
                                if (std::string token; !(tokens >> token)) {
                                  validate(board, blackToMove, castlingOrigins,
                                           enPassantTarget);
                                  problems.push_back(
                                      {.type = ProblemType::PERFT,
                                       .position = {board, blackToMove,
                                                    castlingOrigins,
                                                    enPassantTarget},
                                       .nPlies = nPlies});
                                  continue;
                                }
                              }
                            }
                          } else if (token == "dm") {
                            if (std::string token; tokens >> token) {
                              if (std::regex_match(token,
                                                   std::regex("[1-9]\\d*;"))) {
                                int nMoves = std::stoi(token);
                                if (std::string token; !(tokens >> token)) {
                                  validate(board, blackToMove, castlingOrigins,
                                           enPassantTarget);
                                  problems.push_back(
                                      {.type = ProblemType::MATE_SEARCH,
                                       .position = {board, blackToMove,
                                                    castlingOrigins,
                                                    enPassantTarget},
                                       .nMoves = nMoves});
                                  continue;
                                }
                              }
                            }
                          }
                        } catch (const std::invalid_argument& error) {
                          logger(std::cerr) << "Not accepted line: '" + line +
                                                   "'. " + error.what() + ".\n";
                          return {};
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      logger(std::cerr) << "Invalid line: '" + line + "'.\n";
      return {};
    }
  }
  return problems;
}

void write(const Problem& problem) {
  std::cout << std::string(42, '_') << std::endl;
  std::stringstream output;
  for (int rank = 7; rank >= 0; --rank) {
    output << (rank + 1);
    for (int file = 0; file <= 7; ++file) {
      output << ' ';
      if (std::map<Square, Piece>::const_iterator entry =
              problem.position.board.find({file, rank});
          entry != problem.position.board.cend()) {
        switch (entry->second.type) {
          case PieceType::KING:
            output << (entry->second.black ? 'k' : 'K');
            break;
          case PieceType::QUEEN:
            output << (entry->second.black ? 'q' : 'Q');
            break;
          case PieceType::ROOK:
            output << (entry->second.black ? 'r' : 'R');
            break;
          case PieceType::BISHOP:
            output << (entry->second.black ? 'b' : 'B');
            break;
          case PieceType::KNIGHT:
            output << (entry->second.black ? 'n' : 'N');
            break;
          case PieceType::PAWN:
            output << (entry->second.black ? 'p' : 'P');
            break;
        }
      } else {
        output << '.';
      }
    }
    switch (rank) {
      case 7:
        output << "    Side to move: "
               << (problem.position.blackToMove ? 'b' : 'w');
        break;
      case 6:
        output << "    Castling rights: ";
        if (!problem.position.castlingOrigins.empty()) {
          if (problem.position.castlingOrigins.contains({4, 0})) {
            if (problem.position.castlingOrigins.contains({7, 0})) {
              output << 'K';
            }
            if (problem.position.castlingOrigins.contains({0, 0})) {
              output << 'Q';
            }
          }
          if (problem.position.castlingOrigins.contains({4, 7})) {
            if (problem.position.castlingOrigins.contains({7, 7})) {
              output << 'k';
            }
            if (problem.position.castlingOrigins.contains({0, 7})) {
              output << 'q';
            }
          }
        } else {
          output << '-';
        }
        break;
      case 5:
        output << "    En passant target: ";
        if (problem.position.enPassantTarget) {
          output << static_cast<char>('a' +
                                      problem.position.enPassantTarget->file)
                 << static_cast<char>('1' +
                                      problem.position.enPassantTarget->rank);
        } else {
          output << '-';
        }
        break;
      case 3:
        output << "    ";
        switch (problem.type) {
          case ProblemType::PERFT:
            output << "Perft at depth " << problem.nPlies.value();
            break;
          case ProblemType::MATE_SEARCH:
            output << "Mate in " << problem.nMoves.value();
            break;
        }
        break;
    }
    output << '\n';
  }
  output << ' ';
  for (char file = 'a'; file <= 'h'; ++file) {
    output << ' ' << file;
  }
  std::cout << output.str() << std::endl;
  std::cout << std::endl;
}

}  // namespace pamphlet
