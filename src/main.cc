#include "cards.h"
#include "prompt.h"
#include "span.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <iostream>
#include <map>
#include <random>
#include <thread>

using namespace std::literals;

constexpr int kStartingChips = 50;
constexpr int kMinimumBet = 1;

constexpr const char* kFunnyAiNames[] = {
  "Smenge",
  "Boddit",
  "Ralphus",
  "Dilpo",
  "Blorphus",
  "Smarticus",
  "Klumph",
};

struct Player {
  enum Type {
    kAi,
    kHuman,
  };
  Type type;
  std::string name;
  int chips = kStartingChips;
};

struct Hand {
  Player* owner;
  std::vector<Card> cards;
  int chips = 0;
};

enum class Action {
  kSplit,
  kStick,
  kTwist,
};

std::istream& operator>>(std::istream& input, Action& action) {
  struct Entry {
    std::string_view name;
    Action action;
  };
  static constexpr Entry kNames[] = {
    {"split", Action::kSplit},
    {"stick", Action::kStick},
    {"twist", Action::kTwist},
  };
  std::string name;
  input >> name;
  for (char c : name) c = std::tolower(c);
  auto i = std::find_if(std::begin(kNames), std::end(kNames),
                        [&](const auto& entry) { return entry.name == name; });
  if (i == std::end(kNames)) {
    input.setstate(std::ios_base::failbit);
  } else {
    action = i->action;
  }
  return input;
}

std::ostream& operator<<(std::ostream& output, Action action) {
  switch (action) {
    case Action::kSplit: return output << "split";
    case Action::kStick: return output << "stick";
    case Action::kTwist: return output << "twist";
  }
}

std::ostream& operator<<(std::ostream& output, const Player& player) nobugs {
  // eg. bob (chips: 4000)
  return output << player.name << " (chips: " << player.chips << ")";
}

std::ostream& operator<<(std::ostream& output, const Hand& hand) {
  // eg. bob (chips: 4000) [2H 4S] (current bet: 5)
  return output << *hand.owner << " " << hand.cards
                << " current bet: " << hand.chips;
}

static std::random_device random_device;
static std::mt19937 generator{random_device()};

std::vector<Card> CreateDeck() {
  // Set the deck of cards.
  std::vector<Card> cards;
  cards.reserve(52);
  for (Suit suit : kSuits) {
    for (Rank rank : kRanks) {
      cards.push_back(Card{rank, suit});
    }
  }

  // Shuffle the deck.
  std::shuffle(std::begin(cards), std::end(cards), generator);
  return cards;
}

Card Deal(std::vector<Card>& deck) {
  assert(!deck.empty());
  Card card = deck.back();
  deck.pop_back();
  return card;
}

bool IsValidBet(const Player& player, int bet) {
  assert(player.chips >= kMinimumBet);
  if (bet < kMinimumBet) {
    std::cout << "Your bet is less than the minimum.\n";
    return false;
  } else if (bet > player.chips) {
    std::cout << "You don't have enough chips.\n";
    return false;
  }
  return true;
}

int GetBet(const Hand& hand) {
  if (hand.owner->type == Player::kHuman) {
    std::cout << hand << "\n";
    return Prompt<int>(hand.owner->name + ", place your bet:",
                       [&](int bet) { return IsValidBet(*hand.owner, bet); });
  } else {
    return 1;
  }
}

bool IsValidAction(const Hand& hand, Action action) {
  assert(hand.cards.size() >= 2);
  assert(GetScore(hand.cards) < Score{21} &&
         GetScore(hand.cards) != Score{Bust{}});
  if (action == Action::kSplit) {
    if (hand.cards.size() != 2) {
      std::cout << "You can't split after getting extra cards.\n";
      return false;
    } else if (hand.cards[0].rank != hand.cards[1].rank) {
      std::cout << "Your cards must match for you to split.\n";
      return false;
    } else if (hand.owner->chips < hand.chips) {
      std::cout << "You do not have enough chips to split.\n";
      return false;
    }
  }
  return true;
}

Action GetAction(const Hand& hand) {
  if (hand.owner->type == Player::kHuman) {
    std::cout << hand << "\n";
    return Prompt<Action>(
        hand.owner->name + ", what would you like to do?",
        [&](Action bet) { return IsValidAction(hand, bet); });
  } else {
    std::this_thread::sleep_for(2s);
    return Action::kStick;
  }
}

void PrintHands(nonstd::span<const Hand> hands) {
  std::cout << "-- Current hands --\n";
  for (const auto& hand : hands) std::cout << hand << "\n";
}

struct Units {
  int value;
  std::string_view unit;
};

std::ostream& operator<<(std::ostream& output, Units units) {
  return output << units.value << " " << units.unit
                << (units.value == 1 ? "" : "s");
}

void HandleRound(nonstd::span<Player> players) {
  std::vector<Card> deck = CreateDeck();

  // Phase 1: Hand out cards and shit. Mostly cards. In fact, there's no shit.
  std::vector<Hand> hands;
  for (Player& player : players)
    hands.push_back(Hand{&player, {Deal(deck)}});
  std::vector<Card> dealer_cards = {Deal(deck)};

  PrintHands(hands);
  std::cout << "Dealer: " << dealer_cards << "\n";

  // Phase 2: Get bets for each hand.
  for (Hand& hand : hands) {
    std::cout << "--\n";
    int bet = GetBet(hand);
    std::cout << hand.owner->name << " bets " << bet << "\n";
    hand.owner->chips -= bet;
    hand.chips += bet;
  }

  // Phase 3: Deal the second card.
  for (Hand& hand : hands)
    hand.cards.push_back(Deal(deck));
  dealer_cards.push_back(Deal(deck));

  PrintHands(hands);

  if (IsBlackjack(dealer_cards)) {
    std::cout << "Dealer: " << dealer_cards << "\n";
    std::cout << "Dealer wins!\n";
    return;
  }

  std::cout << "Dealer: " << dealer_cards.front() << " " << kWildcard << "\n";

  // Phase 4: Player actions.
  for (std::size_t i = 0; i < hands.size(); i++) {
    std::cout << "--\n";
    if (IsBlackjack(hands[i].cards)) {
      std::cout << hands[i].owner->name << " has blackjack.\n";
      continue;
    }
    while (true) {
      Action action = GetAction(hands[i]);
      if (action == Action::kStick) {
        std::cout << hands[i].owner->name << " opted to stick with "
                  << hands[i].cards << "\n";
        break;
      }
      std::cout << hands[i].owner->name << " opted to " << action << "\n";
      switch (action) {
        case Action::kStick:
          assert(false);
          break;
        case Action::kSplit:
          assert(hands[i].cards.size() == 2);
          // Double the bet to pay for the second hand.
          hands[i].owner->chips -= hands[i].chips;
          // Add the second hand.
          hands.insert(hands.begin() + i + 1,
                       Hand{hands[i].owner,
                            {hands[i].cards[1], Deal(deck)},
                            hands[i].chips});
          hands[i].cards[1] = Deal(deck);
          break;
        case Action::kTwist:
          hands[i].cards.push_back(Deal(deck));
          std::cout << hands[i].owner->name << " twisted "
                    << hands[i].cards.back() << "\n";
          break;
      }
      if (GetScore(hands[i].cards) == Score{21}) {
        std::cout << hands[i].owner->name << " has 21.\n";
        break;
      }
      if (GetScore(hands[i].cards) == Score{Bust{}}) {
        std::cout << hands[i].owner->name << " has bust.\n";
        break;
      }
    }
  }

  // Phase 5: Dealer acts.
  std::cout << "--\n";
  std::cout << "Dealer reveals their cards: " << dealer_cards << "\n";
  std::this_thread::sleep_for(3s);

  for (Score dealer_score = GetScore(dealer_cards);
       dealer_score != Score{Bust{}} && dealer_score < Score{17};
       dealer_score = GetScore(dealer_cards)) {
    dealer_cards.push_back(Deal(deck));
    std::cout << "Dealer twisted " << dealer_cards.back() << "\n";
    std::this_thread::sleep_for(1s);
  }
  Score dealer_score = GetScore(dealer_cards);
  std::cout << "Dealer " << (dealer_score == Score{Bust{}} ? "bust" : "sticks")
            << " with " << dealer_cards << "\n";
  std::this_thread::sleep_for(1s);

  // Phase 6: Payouts.
  std::cout << "--\n";
  std::map<Player*, int> player_winnings;
  for (Hand& hand : hands) {
    if (dealer_score < GetScore(hand.cards)) {
      player_winnings[hand.owner] += hand.chips;
      hand.owner->chips += 2 * hand.chips;
    } else {
      player_winnings[hand.owner] -= hand.chips;
    }
  }
  for (const auto& [player, winnings] : player_winnings) {
    std::cout << player->name << " ";
    if (winnings == 0) {
      std::cout << "broke even";
    } else if (winnings < 0) {
      std::cout << "lost " << Units{-winnings, "chip"};
    } else {
      std::cout << "won " << Units{winnings, "chip"};
    }
    std::cout << ".\n";
  }
}

int main() {
  // Set up the players.
  std::vector<Player> players;

  int num_humans = Prompt<int>("Number of human players?",
                               [](int x) { return x >= 0; });
  int num_ai_players = Prompt<int>("Number of AI players?", [](int x) {
    return x >= 0 && x <= static_cast<int>(std::size(kFunnyAiNames));
  });

  if (num_humans + num_ai_players == 0) {
    std::cout << "No.\n";
    return 1;
  }

  // Human players.
  std::map<std::string, int> names;
  for (int i = 0; i < num_humans; i++) {
    std::cout << "Enter a name for human player #" << (i + 1) << ": ";
    std::string name;
    std::getline(std::cin, name);
    int count = ++names[name];
    if (count > 1) name += " #" + std::to_string(count);
    players.push_back(Player{Player::kHuman, std::move(name)});
  }

  // AI players.
  std::vector<std::string_view> ai_names;
  std::sample(std::begin(kFunnyAiNames), std::end(kFunnyAiNames),
              std::back_inserter(ai_names), num_ai_players, generator);
  for (int i = 0; i < num_ai_players; i++) {
    players.push_back(
        Player{Player::kAi, std::string{ai_names[i]}, kStartingChips});
  }

  while (!players.empty()) {
    // Shuffle the players.
    std::shuffle(std::begin(players), std::end(players), generator);
    HandleRound(players);
    // Remove any players which have no chips left.
    auto new_end =
        std::partition(std::begin(players), std::end(players),
                       [](const auto& player) { return player.chips > 0; });
    for (auto i = new_end; i != std::end(players); i++) {
      std::cout << i->name << " is broke.\n";
    }
    players.erase(new_end, std::end(players));
    std::this_thread::sleep_for(5s);
  }
}
