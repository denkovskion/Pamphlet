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

#include "Move.h"

namespace pamphlet {

Position doMake(const Move& move, Position position);

std::optional<Position> make(
    const Move& move, const Position& position,
    std::optional<std::reference_wrapper<std::vector<Move>>> pseudoLegalMoves,
    std::optional<std::reference_wrapper<std::ostream>> lanBuilder) {
  bool preLegal = false;
  switch (move.type) {
    case MoveType::NULL_MOVE:
      preLegal = true;
      break;
    case MoveType::QUIET_MOVE:
      preLegal = true;
      break;
    case MoveType::CAPTURE:
      preLegal = true;
      break;
    case MoveType::LONG_CASTLING:
      if (make({.type = MoveType::NULL_MOVE}, position, std::nullopt,
               std::nullopt)) {
        if (make({.type = MoveType::QUIET_MOVE,
                  .origin = move.origin,
                  .target = move.target2},
                 position, std::nullopt, std::nullopt)) {
          preLegal = true;
        }
      }
      break;
    case MoveType::SHORT_CASTLING:
      if (make({.type = MoveType::NULL_MOVE}, position, std::nullopt,
               std::nullopt)) {
        if (make({.type = MoveType::QUIET_MOVE,
                  .origin = move.origin,
                  .target = move.target2},
                 position, std::nullopt, std::nullopt)) {
          preLegal = true;
        }
      }
      break;
    case MoveType::DOUBLE_STEP:
      preLegal = true;
      break;
    case MoveType::EN_PASSANT:
      preLegal = true;
      break;
    case MoveType::PROMOTION:
      preLegal = true;
      break;
    case MoveType::PROMOTION_CAPTURE:
      preLegal = true;
      break;
  }
  if (preLegal) {
    Position result = doMake(move, position);
    if (isLegal(result, pseudoLegalMoves)) {
      if (lanBuilder) {
        switch (move.type) {
          case MoveType::NULL_MOVE:
            lanBuilder->get() << "null";
            break;
          case MoveType::QUIET_MOVE:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "-"
                << toLanCode(move.target.value());
            break;
          case MoveType::CAPTURE:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "x"
                << toLanCode(move.target.value());
            break;
          case MoveType::LONG_CASTLING:
            lanBuilder->get() << "0-0-0";
            break;
          case MoveType::SHORT_CASTLING:
            lanBuilder->get() << "0-0";
            break;
          case MoveType::DOUBLE_STEP:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "-"
                << toLanCode(move.target.value());
            break;
          case MoveType::EN_PASSANT:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "x"
                << toLanCode(move.target.value()) << " e.p.";
            break;
          case MoveType::PROMOTION:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "-"
                << toLanCode(move.target.value()) << "="
                << toLanCode(move.promoted.value());
            break;
          case MoveType::PROMOTION_CAPTURE:
            lanBuilder->get()
                << toLanCode(position.board.at(move.origin.value()))
                << toLanCode(move.origin.value()) << "x"
                << toLanCode(move.target.value()) << "="
                << toLanCode(move.promoted.value());
            break;
        }
        std::vector<Move> pseudoLegalMovesNext;
        if (pseudoLegalMoves) {
          pseudoLegalMovesNext = pseudoLegalMoves->get();
        } else {
          generateMoves(result.board, result.blackToMove,
                        result.castlingOrigins, result.enPassantTarget,
                        pseudoLegalMovesNext, true);
        }
        bool terminal = true;
        for (const Move& moveNext : pseudoLegalMovesNext) {
          if (make(moveNext, result, std::nullopt, std::nullopt)) {
            terminal = false;
            break;
          }
        }
        Position opposite = doMake({.type = MoveType::NULL_MOVE}, result);
        int legal = generateMoves(opposite.board, opposite.blackToMove,
                                  opposite.castlingOrigins,
                                  opposite.enPassantTarget, std::nullopt, true);
        if (terminal) {
          if (legal == 1) {
            lanBuilder->get() << "=";
          } else {
            if (legal < -1) {
              lanBuilder->get() << std::string(-legal, '+');
            }
            lanBuilder->get() << "#";
          }
        } else {
          if (legal < 0) {
            lanBuilder->get() << std::string(-legal, '+');
          }
        }
      }
      return result;
    }
  }
  return std::nullopt;
}

Position doMake(const Move& move, Position position) {
  switch (move.type) {
    case MoveType::NULL_MOVE:
      position.enPassantTarget.reset();
      break;
    case MoveType::QUIET_MOVE:
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.castlingOrigins.erase(move.origin.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::CAPTURE:
      position.board.erase(move.target.value());
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.castlingOrigins.erase(move.origin.value());
      position.castlingOrigins.erase(move.target.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::LONG_CASTLING:
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.board.insert(
          {move.target2.value(), position.board.at(move.origin2.value())});
      position.board.erase(move.origin2.value());
      position.castlingOrigins.erase(move.origin.value());
      position.castlingOrigins.erase(move.origin2.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::SHORT_CASTLING:
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.board.insert(
          {move.target2.value(), position.board.at(move.origin2.value())});
      position.board.erase(move.origin2.value());
      position.castlingOrigins.erase(move.origin.value());
      position.castlingOrigins.erase(move.origin2.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::DOUBLE_STEP:
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.enPassantTarget = move.stop.value();
      break;
    case MoveType::EN_PASSANT:
      position.board.erase(move.stop.value());
      position.board.insert(
          {move.target.value(), position.board.at(move.origin.value())});
      position.board.erase(move.origin.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::PROMOTION:
      position.board.insert({move.target.value(), move.promoted.value()});
      position.board.erase(move.origin.value());
      position.enPassantTarget.reset();
      break;
    case MoveType::PROMOTION_CAPTURE:
      position.board.erase(move.target.value());
      position.board.insert({move.target.value(), move.promoted.value()});
      position.board.erase(move.origin.value());
      position.castlingOrigins.erase(move.target.value());
      position.enPassantTarget.reset();
      break;
  }
  position.blackToMove = !position.blackToMove;
  return position;
}

}  // namespace pamphlet
