#pragma once

#include "span.h"

#include <algorithm>
#include <iosfwd>
#include <variant>

enum class Suit : unsigned char {
  kWildcard,
  kClubs,
  kDiamonds,
  kHearts,
  kSpades,
};

constexpr Suit kSuits[] = {Suit::kHearts, Suit::kClubs, Suit::kDiamonds,
                           Suit::kSpades};

std::ostream& operator<<(std::ostream& output, Suit suit);

enum class Rank : unsigned char {
  // Make the card values start at 1 rather than 0 so that all the number values
  // match up with their name.
  kWildcard,
  kAce,
  kTwo,
  kThree,
  kFour,
  kFive,
  kSix,
  kSeven,
  kEight,
  kNine,
  kTen,
  kJack,
  kQueen,
  kKing,
};

constexpr Rank kRanks[] = {Rank::kAce,  Rank::kTwo, Rank::kThree, Rank::kFour,
                           Rank::kFive, Rank::kSix, Rank::kSeven, Rank::kEight,
                           Rank::kNine, Rank::kTen, Rank::kJack,  Rank::kQueen,
                           Rank::kKing};

constexpr bool IsPicture(Rank rank);

// Return the numeric value of a card, aces low.
constexpr int Valueof(Rank rank);

std::ostream& operator<<(std::ostream& output, Rank card);

struct Card {
  Rank rank;
  Suit suit;
};
static constexpr Card kWildcard = Card{Rank::kWildcard, Suit::kWildcard};

constexpr bool operator==(Card left, Card right) {
  return left.rank == right.rank && left.suit == right.suit;
}

constexpr bool operator!=(Card left, Card right) {
  return !(left == right);
}

std::ostream& operator<<(std::ostream& output, Card card);
std::ostream& operator<<(std::ostream& output, nonstd::span<const Card> cards);

// A hand is either a blackjack or it is not. If it is not, we need the numeric
// total of the hand for comparing it against the dealer's hand.
using Total = int;
enum class Bust {};
enum class Blackjack {};
using Score = std::variant<Bust, Total, Blackjack>;
std::ostream& operator<<(std::ostream& output, Bust);
std::ostream& operator<<(std::ostream& output, Blackjack);
std::ostream& operator<<(std::ostream& output, Score score);

constexpr bool IsPicture(Rank rank) {
  return rank == Rank::kJack || rank == Rank::kQueen || rank == Rank::kKing;
}

constexpr int ValueOf(Rank rank) {
  // All ranks except picture card ranks have the right value as their
  // representation, so we just need to adjust the picture cards.
  return IsPicture(rank) ? 10 : static_cast<int>(rank);
}

constexpr Score GetScore(nonstd::span<const Card> cards) {
  bool has_ace = false;
  Total total = 0;
  for (Card card : cards) {
    if (card.rank == Rank::kAce) has_ace = true;
    total += ValueOf(card.rank);
  }
  if (total > 21) return Bust{};
  if (total <= 11 && has_ace) total += 10;
  if (cards.size() == 2 && total == 21) return Blackjack{};
  return total;
}

constexpr bool IsBlackjack(nonstd::span<const Card> cards) {
  return GetScore(cards) == Score{Blackjack{}};
}
