#include "cards.h"

#include <iostream>

std::ostream& operator<<(std::ostream& output, Suit suit) {
  switch (suit) {
    case Suit::kWildcard: return output << "?";
    case Suit::kClubs: return output << "♣";
    case Suit::kDiamonds: return output << "♦";
    case Suit::kHearts: return output << "♥";
    case Suit::kSpades: return output << "♠";
  }
}

std::ostream& operator<<(std::ostream& output, Rank rank) {
  switch (rank) {
    case Rank::kWildcard: return output << "?";
    case Rank::kAce: return output << "A";
    case Rank::kJack: return output << "J";
    case Rank::kQueen: return output << "Q";
    case Rank::kKing: return output << "K";
    default: return output << static_cast<int>(rank);  // Rank is a number.
  }
}

namespace {

constexpr char kWhiteBackground[] = "\x1b[47m";
constexpr char kRedBackground[] = "\x1b[41m";
constexpr char kWhite[] = "\x1b[37;1m";
constexpr char kRed[] = "\x1b[31m";
constexpr char kBlack[] = "\x1b[30m";
constexpr char kReset[] = "\x1b[0m";

constexpr const char* Style(Card card) noexcept {
  switch (card.suit) {
    case Suit::kWildcard:
      return kWhite;
    case Suit::kDiamonds:
    case Suit::kHearts:
      return kRed;
    case Suit::kClubs:
    case Suit::kSpades:
      return kBlack;
  }
}

}  // namespace

std::ostream& operator<<(std::ostream& output, Bust) {
  return output << "bust";
}

std::ostream& operator<<(std::ostream& output, Blackjack) {
  return output << "blackjack";
}

std::ostream& operator<<(std::ostream& output, Score score) {
  std::visit([&](auto value) { output << value; }, score);
  return output;
}

std::ostream& operator<<(std::ostream& output, Card card) {
  if (card == kWildcard) {
    output << kRedBackground;
  } else {
    output << kWhiteBackground;
  }
  return output << Style(card) << card.rank << card.suit << kReset;
}

std::ostream& operator<<(std::ostream& output, nonstd::span<const Card> cards) {
  bool first = true;
  for (Card card : cards) {
    if (first) {
      first = false;
    } else {
      output << " ";
    }
    output << card;
  }
  output << " (";
  if (IsBlackjack(cards)) {
    output << "blackjack";
  } else {
    output << "total: " << GetScore(cards);
  }
  return output << ")";
}
