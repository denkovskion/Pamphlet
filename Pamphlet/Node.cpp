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

#include "Node.h"

#include <sstream>

namespace pamphlet {

std::string toFormatted(const Node& node, const Position& position, int moveNo,
                        bool inlyne) {
  std::stringstream output;
  if (node.type == NodeType::ILLEGAL_NODE) {
    output << "Illegal position";
  } else if (node.type == NodeType::DIVIDE_ROOT) {
    for (const Node& child : node.children.value()) {
      output << toFormatted(child, position, moveNo, false) << "\n";
    }
    output << node.count.value();
  } else if (node.type == NodeType::DIVIDE_LEAF) {
    make(node.move.value(), position, std::nullopt, output);
    output << " " << node.count.value();
  } else if (node.type == NodeType::PERFT_NODE) {
    output << node.count.value();
  } else if (node.type == NodeType::MATE_ROOT) {
    for (bool first = true; const Node& child : node.children.value()) {
      if (!first) {
        output << "\n";
      }
      output << toFormatted(child, position, moveNo, false);
      first = false;
    }
  } else if (node.type == NodeType::MATE_BRANCH) {
    if (position.blackToMove) {
      if (!inlyne) {
        output << moveNo << "...";
      }
    } else {
      output << moveNo << ".";
    }
    std::optional<Position> positionNext =
        make(node.move.value(), position, std::nullopt, output);
    for (bool first = true; const Node& child : node.children.value()) {
      if (first) {
        output << " ";
      } else {
        output << "\n";
        for (int i = 0;
             i < (positionNext.value().blackToMove ? moveNo - 1 : moveNo);
             ++i) {
          output << "\t";
        }
      }
      output << toFormatted(
          child, positionNext.value(),
          positionNext.value().blackToMove ? moveNo : moveNo + 1, first);
      first = false;
    }
  } else if (node.type == NodeType::MATE_LEAF) {
    make(node.move.value(), position, std::nullopt, output);
    output << " [#" << node.distance.value() << "]";
  }
  return output.str();
}

}  // namespace pamphlet
