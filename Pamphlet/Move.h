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

#pragma once

#include <ostream>

#include "Position.h"

namespace pamphlet {

enum class MoveType {
  NULL_MOVE,
  QUIET_MOVE,
  CAPTURE,
  LONG_CASTLING,
  SHORT_CASTLING,
  DOUBLE_STEP,
  EN_PASSANT,
  PROMOTION,
  PROMOTION_CAPTURE
};

struct Move {
  MoveType type;
  std::optional<Square> origin;
  std::optional<Square> target;
  std::optional<Square> origin2;
  std::optional<Square> target2;
  std::optional<Square> stop;
  std::optional<Piece> promoted;
};

std::optional<Position> make(
    const Move& move, const Position& position,
    std::optional<std::reference_wrapper<std::vector<Move>>> pseudoLegalMoves,
    std::optional<std::reference_wrapper<std::ostream>> lanBuilder);

}  // namespace pamphlet
