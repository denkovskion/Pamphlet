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

#include <algorithm>
#include <span>
#include <stdexcept>
#include <utility>

#include "Move.h"

namespace pamphlet {

struct Direction {
  int fileOffset;
  int rankOffset;

  std::strong_ordering operator<=>(const Direction& other) const = default;
};

std::vector<Direction> computeDirections(const std::set<Direction>& bases);

std::string toLanCode(const Square& square) {
  return std::string()
      .append(1, 'a' + square.file - 1)
      .append(1, '1' + square.rank - 1);
}

const char* toLanCode(const Piece& piece) {
  switch (piece.type) {
    case PieceType::KING:
      return "K";
    case PieceType::QUEEN:
      return "Q";
    case PieceType::ROOK:
      return "R";
    case PieceType::BISHOP:
      return "B";
    case PieceType::KNIGHT:
      return "N";
    case PieceType::PAWN:
      return "";
  }
  return nullptr;
}

void validate(const std::map<Square, Piece>& board, bool blackToMove,
              const std::set<Square>& castlingOrigins,
              const std::optional<Square>& enPassantTarget) {
  for (bool black : {false, true}) {
    if (!(std::count_if(board.cbegin(), board.cend(),
                        [black](const std::pair<Square, Piece>& entry) {
                          return entry.second.black == black &&
                                 entry.second.type == PieceType::KING;
                        }) == 1)) {
      throw std::invalid_argument("Not accepted number of kings");
    }
  }
  for (const Square& castlingOrigin : castlingOrigins) {
    if (std::map<Square, Piece>::const_iterator entry =
            board.find(castlingOrigin);
        !(entry != board.cend() &&
          (castlingOrigin.file == 5 && entry->second.type == PieceType::KING ||
           (castlingOrigin.file == 1 || castlingOrigin.file == 8) &&
               entry->second.type == PieceType::ROOK) &&
          (castlingOrigin.rank == 1 && !entry->second.black ||
           castlingOrigin.rank == 8 && entry->second.black))) {
      throw std::invalid_argument("Not accepted castling rights");
    }
  }
  if (enPassantTarget) {
    if (std::map<Square, Piece>::const_iterator entry =
            board.find({enPassantTarget->file, blackToMove ? 4 : 5});
        !(enPassantTarget->rank == (blackToMove ? 3 : 6) &&
          entry != board.cend() && entry->second.type == PieceType::PAWN &&
          entry->second.black != blackToMove &&
          !board.contains(enPassantTarget.value()) &&
          !board.contains({enPassantTarget->file, blackToMove ? 2 : 7}))) {
      throw std::invalid_argument("Not accepted en passant square");
    }
  }
}

int generateMoves(
    const std::map<Square, Piece>& board, bool blackToMove,
    const std::set<Square>& castlingOrigins,
    const std::optional<Square>& enPassantTarget,
    std::optional<std::reference_wrapper<std::vector<Move>>> moves,
    bool count) {
  int nChecks = 0;
  for (const std::pair<Square, Piece>& entry : board) {
    if (const Piece& piece = entry.second; piece.black == blackToMove) {
      const Square& origin = entry.first;
      if (piece.type == PieceType::KING || piece.type == PieceType::KNIGHT ||
          piece.type == PieceType::QUEEN || piece.type == PieceType::ROOK ||
          piece.type == PieceType::BISHOP) {
        std::span<const Direction> directions;
        if (piece.type == PieceType::KING || piece.type == PieceType::QUEEN) {
          static const std::vector<Direction> offsets =
              computeDirections({{0, 1}, {1, 1}});
          directions = offsets;
        } else if (piece.type == PieceType::ROOK) {
          static const std::vector<Direction> offsets =
              computeDirections({{0, 1}});
          directions = offsets;
        } else if (piece.type == PieceType::BISHOP) {
          static const std::vector<Direction> offsets =
              computeDirections({{1, 1}});
          directions = offsets;
        } else if (piece.type == PieceType::KNIGHT) {
          static const std::vector<Direction> offsets =
              computeDirections({{1, 2}});
          directions = offsets;
        }
        for (const Direction& direction : directions) {
          for (Square square = origin;;) {
            if (Square target = {square.file + direction.fileOffset,
                                 square.rank + direction.rankOffset};
                target.file >= 1 && target.file <= 8 && target.rank >= 1 &&
                target.rank <= 8) {
              if (std::map<Square, Piece>::const_iterator other =
                      board.find(target);
                  other != board.cend()) {
                if (const Piece& captured = other->second;
                    captured.black != piece.black) {
                  if (captured.type == PieceType::KING) {
                    if (count) {
                      ++nChecks;
                    } else {
                      return 0;
                    }
                  }
                  if (moves) {
                    moves->get().push_back({.type = MoveType::CAPTURE,
                                            .origin = origin,
                                            .target = target});
                  }
                }
                break;
              } else {
                if (moves) {
                  moves->get().push_back({.type = MoveType::QUIET_MOVE,
                                          .origin = origin,
                                          .target = target});
                }
                if (piece.type == PieceType::KING ||
                    piece.type == PieceType::KNIGHT) {
                  break;
                }
              }
              square = target;
            } else {
              break;
            }
          }
        }
        if (piece.type == PieceType::KING) {
          if (castlingOrigins.contains(origin)) {
            static const Direction castlingDirections[] = {{-1, 0}, {1, 0}};
            for (const Direction& direction : castlingDirections) {
              Square target2 = {origin.file + direction.fileOffset,
                                origin.rank + direction.rankOffset};
              if (!board.contains(target2)) {
                Square target = {target2.file + direction.fileOffset,
                                 target2.rank + direction.rankOffset};
                if (!board.contains(target)) {
                  if (direction.fileOffset > 0) {
                    Square origin2 = {target.file + direction.fileOffset,
                                      target.rank + direction.rankOffset};
                    if (castlingOrigins.contains(origin2)) {
                      if (moves) {
                        moves->get().push_back(
                            {.type = MoveType::SHORT_CASTLING,
                             .origin = origin,
                             .target = target,
                             .origin2 = origin2,
                             .target2 = target2});
                      }
                    }
                  } else {
                    Square stop = {target.file + direction.fileOffset,
                                   target.rank + direction.rankOffset};
                    if (!board.contains(stop)) {
                      Square origin2 = {stop.file + direction.fileOffset,
                                        stop.rank + direction.rankOffset};
                      if (castlingOrigins.contains(origin2)) {
                        if (moves) {
                          moves->get().push_back(
                              {.type = MoveType::LONG_CASTLING,
                               .origin = origin,
                               .target = target,
                               .origin2 = origin2,
                               .target2 = target2});
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      } else if (piece.type == PieceType::PAWN) {
        std::span<const Direction> captureDirections;
        if (piece.black) {
          static const Direction offsets[] = {{-1, -1}, {1, -1}};
          captureDirections = offsets;
        } else {
          static const Direction offsets[] = {{-1, 1}, {1, 1}};
          captureDirections = offsets;
        }
        for (const Direction& direction : captureDirections) {
          if (Square target = {origin.file + direction.fileOffset,
                               origin.rank + direction.rankOffset};
              target.file >= 1 && target.file <= 8 && target.rank >= 1 &&
              target.rank <= 8) {
            if (std::map<Square, Piece>::const_iterator other =
                    board.find(target);
                other != board.cend()) {
              if (const Piece& captured = other->second;
                  captured.black != piece.black) {
                if (captured.type == PieceType::KING) {
                  if (count) {
                    ++nChecks;
                  } else {
                    return 0;
                  }
                }
                if (origin.rank == (piece.black ? 2 : 7)) {
                  Piece pieces[] = {{PieceType::QUEEN, piece.black},
                                    {PieceType::ROOK, piece.black},
                                    {PieceType::BISHOP, piece.black},
                                    {PieceType::KNIGHT, piece.black}};
                  for (const Piece& promoted : pieces) {
                    if (moves) {
                      moves->get().push_back(
                          {.type = MoveType::PROMOTION_CAPTURE,
                           .origin = origin,
                           .target = target,
                           .promoted = promoted});
                    }
                  }
                } else {
                  if (moves) {
                    moves->get().push_back({.type = MoveType::CAPTURE,
                                            .origin = origin,
                                            .target = target});
                  }
                }
              }
            } else {
              if (enPassantTarget) {
                if (target == enPassantTarget.value()) {
                  Square stop = {target.file, origin.rank};
                  if (moves) {
                    moves->get().push_back({.type = MoveType::EN_PASSANT,
                                            .origin = origin,
                                            .target = target,
                                            .stop = stop});
                  }
                }
              }
            }
          }
        }
        Direction direction = {0, piece.black ? -1 : 1};
        if (Square target = {origin.file + direction.fileOffset,
                             origin.rank + direction.rankOffset};
            target.file >= 1 && target.file <= 8 && target.rank >= 1 &&
            target.rank <= 8) {
          if (!board.contains(target)) {
            if (origin.rank == (piece.black ? 2 : 7)) {
              Piece pieces[] = {{PieceType::QUEEN, piece.black},
                                {PieceType::ROOK, piece.black},
                                {PieceType::BISHOP, piece.black},
                                {PieceType::KNIGHT, piece.black}};
              for (const Piece& promoted : pieces) {
                if (moves) {
                  moves->get().push_back({.type = MoveType::PROMOTION,
                                          .origin = origin,
                                          .target = target,
                                          .promoted = promoted});
                }
              }
            } else {
              if (moves) {
                moves->get().push_back({.type = MoveType::QUIET_MOVE,
                                        .origin = origin,
                                        .target = target});
              }
              if (origin.rank == (piece.black ? 7 : 2)) {
                Square target2 = {target.file + direction.fileOffset,
                                  target.rank + direction.rankOffset};
                if (!board.contains(target2)) {
                  if (moves) {
                    moves->get().push_back({.type = MoveType::DOUBLE_STEP,
                                            .origin = origin,
                                            .target = target2,
                                            .stop = target});
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return nChecks == 0 ? 1 : -nChecks;
}

std::vector<Direction> computeDirections(const std::set<Direction>& bases) {
  std::set<Direction> directions;
  for (const Direction& base : bases) {
    for (int fileOffset : {-base.fileOffset, base.fileOffset}) {
      for (int rankOffset : {-base.rankOffset, base.rankOffset}) {
        directions.insert({fileOffset, rankOffset});
        directions.insert({rankOffset, fileOffset});
      }
    }
  }
  return std::vector<Direction>(directions.cbegin(), directions.cend());
}

}  // namespace pamphlet
