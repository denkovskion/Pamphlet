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

#include "Problem.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>

#include "Node.h"

namespace pamphlet {

unsigned long long count(int nPlies, const Position& position,
                         const std::vector<Move>& pseudoLegalMoves,
                         bool verbose);
int search(int nMoves, const Position& position,
           const std::vector<Move>& pseudoLegalMoves);

void solve(const Problem& problem, bool verbose) {
  logger(std::clog) << "Solving...\n";
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  std::vector<Move> pseudoLegalMoves;
  if (isLegal(problem.position, pseudoLegalMoves)) {
    if (problem.type == ProblemType::PERFT) {
      unsigned long long nNodes = count(
          problem.nPlies.value(), problem.position, pseudoLegalMoves, verbose);
      std::cout << toFormatted({.type = NodeType::PERFT_NODE, .count = nNodes},
                               problem.position)
                << std::endl;
    } else if (problem.type == ProblemType::MATE_SEARCH) {
      std::vector<Node> nodes;
      for (const Move& moveMax : pseudoLegalMoves) {
        std::vector<Move> pseudoLegalMovesMin;
        std::ostringstream lanBuilder;
        if (std::optional<Position> positionMin =
                (verbose ? make(moveMax, problem.position, pseudoLegalMovesMin,
                                lanBuilder)
                         : make(moveMax, problem.position, pseudoLegalMovesMin,
                                std::nullopt))) {
          int depth = 1;
          for (; depth <= problem.nMoves.value(); depth++) {
            if (search(depth, positionMin.value(), pseudoLegalMovesMin) == 1) {
              nodes.push_back({.type = NodeType::MATE_LEAF,
                               .move = moveMax,
                               .distance = depth});
              break;
            }
          }
          if (verbose) {
            logger(std::clog)
                << (depth <= problem.nMoves.value()
                        ? std::format("Tried '{}'. Found mate in {}.\n",
                                      lanBuilder.str(), depth)
                        : std::format("Tried '{}'. No mate in {}.\n",
                                      lanBuilder.str(),
                                      problem.nMoves.value()));
          }
        }
      }
      std::stable_sort(nodes.begin(), nodes.end(),
                       [](const Node& node1, const Node& node2) {
                         return node1.distance.value() < node2.distance.value();
                       });
      std::cout << toFormatted({.type = NodeType::MATE_ROOT, .children = nodes},
                               problem.position)
                << std::endl;
    }
  } else {
    std::cout << toFormatted({.type = NodeType::ILLEGAL_NODE}, problem.position)
              << std::endl;
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  logger(std::clog) << std::format(
      "Finished solving in {}ms.\n",
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count());
}

unsigned long long count(int nPlies, const Position& position,
                         const std::vector<Move>& pseudoLegalMoves,
                         bool verbose) {
  if (nPlies == 0) {
    return 1;
  }
  unsigned long long nNodes = 0;
  for (const Move& move : pseudoLegalMoves) {
    std::vector<Move> pseudoLegalMovesNext;
    std::ostringstream lanBuilder;
    if (std::optional<Position> positionNext =
            (verbose
                 ? make(move, position, pseudoLegalMovesNext, lanBuilder)
                 : make(move, position, pseudoLegalMovesNext, std::nullopt))) {
      unsigned long long nChildNodes =
          count(nPlies - 1, positionNext.value(), pseudoLegalMovesNext, false);
      nNodes += nChildNodes;
      if (verbose) {
        logger(std::clog) << std::format(
            "Evaluated '{}'. Counted {} nodes at depth {}.\n", lanBuilder.str(),
            nChildNodes, nPlies);
      }
    }
  }
  if (verbose) {
    logger(std::clog) << std::format(
        "Finished counting. {} nodes at depth {}.\n", nNodes, nPlies);
  }
  return nNodes;
}

int search(int nMoves, const Position& position,
           const std::vector<Move>& pseudoLegalMoves) {
  int min = 0;
  if (nMoves == 1) {
    for (const Move& moveMin : pseudoLegalMoves) {
      if (make(moveMin, position, std::nullopt, std::nullopt)) {
        min = -1;
        break;
      }
    }
  } else {
    for (const Move& moveMin : pseudoLegalMoves) {
      if (std::vector<Move> pseudoLegalMovesMax;
          std::optional<Position> positionMax =
              make(moveMin, position, pseudoLegalMovesMax, std::nullopt)) {
        int max = -1;
        for (const Move& moveMax : pseudoLegalMovesMax) {
          if (std::vector<Move> pseudoLegalMovesMin;
              std::optional<Position> positionMin =
                  make(moveMax, positionMax.value(), pseudoLegalMovesMin,
                       std::nullopt)) {
            max = search(nMoves - 1, positionMin.value(), pseudoLegalMovesMin);
            if (max == 1) {
              break;
            }
          }
        }
        min = max;
        if (min == -1) {
          break;
        }
      }
    }
  }
  if (min == 0) {
    min = make({.type = MoveType::NULL_MOVE}, position, std::nullopt,
               std::nullopt)
              ? -1
              : 1;
  }
  return min;
}

std::ostream& logger(std::ostream& output) {
  std::time_t calendar = std::time(nullptr);
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
  std::tm* local = std::localtime(&calendar);
#if _MSC_VER
#pragma warning(pop)
#endif
  if (local) {
    output << std::put_time(local, "%c Pamphlet: ");
  } else {
    output << "Pamphlet: ";
  }
  return output;
}

}  // namespace pamphlet
