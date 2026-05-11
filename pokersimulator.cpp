#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>
#include <cmath>

enum Suit {
    Spades,
    Hearts,
    Clubs,
    Diamonds
};

struct Card {
    int rank;
    Suit suit;
};

struct HandValue {
    int category;
    std::vector<int> tiebreakers;
};

struct SimulationResult {
    long long p1Wins = 0;
    long long p2Wins = 0;
    long long ties = 0;
};

bool sameCard(const Card& a, const Card& b) {
    return a.rank == b.rank && a.suit == b.suit;
}

std::string suitToString(Suit s) {
    switch (s) {
    case Spades: return "s";
    case Hearts: return "h";
    case Clubs: return "c";
    case Diamonds: return "d";
    }
    return "?";
}

std::string cardToString(const Card& c) {
    std::string rank;

    if (c.rank >= 2 && c.rank <= 9) rank = std::to_string(c.rank);
    else if (c.rank == 10) rank = "T";
    else if (c.rank == 11) rank = "J";
    else if (c.rank == 12) rank = "Q";
    else if (c.rank == 13) rank = "K";
    else if (c.rank == 14) rank = "A";
    else rank = "?";

    return rank + suitToString(c.suit);
}

Card stringToCard(std::string input) {
    Card c{};
    if (input.size() != 2) {
        c.rank = -1;
        return c;
    }

    switch (input[0]) {
    case 'A': case 'a': c.rank = 14; break;
    case 'K': case 'k': c.rank = 13; break;
    case 'Q': case 'q': c.rank = 12; break;
    case 'J': case 'j': c.rank = 11; break;
    case 'T': case 't': c.rank = 10; break;
    case '9': c.rank = 9; break;
    case '8': c.rank = 8; break;
    case '7': c.rank = 7; break;
    case '6': c.rank = 6; break;
    case '5': c.rank = 5; break;
    case '4': c.rank = 4; break;
    case '3': c.rank = 3; break;
    case '2': c.rank = 2; break;
    default: c.rank = -1; break;
    }

    switch (input[1]) {
    case 's': case 'S': c.suit = Spades; break;
    case 'h': case 'H': c.suit = Hearts; break;
    case 'c': case 'C': c.suit = Clubs; break;
    case 'd': case 'D': c.suit = Diamonds; break;
    default: c.rank = -1; break;
    }

    return c;
}

std::vector<Card> createDeck() {
    std::vector<Card> deck;
    std::vector<Suit> suits = { Spades, Hearts, Clubs, Diamonds };

    for (Suit suit : suits) {
        for (int rank = 2; rank <= 14; rank++) {
            deck.push_back(Card{ rank, suit });
        }
    }

    return deck;
}

void removeCardFromDeck(std::vector<Card>& deck, const Card& card) {
    deck.erase(
        std::remove_if(deck.begin(), deck.end(),
            [&](const Card& c) {
                return sameCard(c, card);
            }),
        deck.end()
    );
}

bool isStraight(const std::vector<int>& ranks, int& highCard) {
    std::vector<int> uniqueRanks = ranks;
    std::sort(uniqueRanks.begin(), uniqueRanks.end());
    uniqueRanks.erase(std::unique(uniqueRanks.begin(), uniqueRanks.end()), uniqueRanks.end());

    if (std::find(uniqueRanks.begin(), uniqueRanks.end(), 14) != uniqueRanks.end()) {
        uniqueRanks.insert(uniqueRanks.begin(), 1);
    }

    int count = 1;
    highCard = -1;

    for (int i = 1; i < (int)uniqueRanks.size(); i++) {
        if (uniqueRanks[i] == uniqueRanks[i - 1] + 1) {
            count++;
            if (count >= 5) {
                highCard = uniqueRanks[i];
            }
        }
        else {
            count = 1;
        }
    }

    return highCard != -1;
}

HandValue evaluate5Cards(std::vector<Card> cards) {
    std::vector<int> ranks;
    int suitCount[4] = { 0 };
    int rankCount[15] = { 0 };

    for (Card c : cards) {
        ranks.push_back(c.rank);
        rankCount[c.rank]++;
        suitCount[c.suit]++;
    }

    std::sort(ranks.begin(), ranks.end(), std::greater<int>());

    bool flush = false;
    for (int i = 0; i < 4; i++) {
        if (suitCount[i] == 5) flush = true;
    }

    int straightHigh = -1;
    bool straight = isStraight(ranks, straightHigh);

    if (straight && flush) {
        return HandValue{ 8, {straightHigh} };
    }

    for (int r = 14; r >= 2; r--) {
        if (rankCount[r] == 4) {
            int kicker = -1;
            for (int k = 14; k >= 2; k--) {
                if (rankCount[k] == 1) {
                    kicker = k;
                    break;
                }
            }
            return HandValue{ 7, {r, kicker} };
        }
    }

    int trips = -1;
    int pair = -1;

    for (int r = 14; r >= 2; r--) {
        if (rankCount[r] == 3 && trips == -1) trips = r;
        else if (rankCount[r] >= 2 && pair == -1) pair = r;
    }

    if (trips != -1 && pair != -1) {
        return HandValue{ 6, {trips, pair} };
    }

    if (flush) {
        return HandValue{ 5, ranks };
    }

    if (straight) {
        return HandValue{ 4, {straightHigh} };
    }

    if (trips != -1) {
        std::vector<int> kickers = { trips };
        for (int r = 14; r >= 2; r--) {
            if (rankCount[r] == 1) kickers.push_back(r);
        }
        return HandValue{ 3, kickers };
    }

    std::vector<int> pairs;
    for (int r = 14; r >= 2; r--) {
        if (rankCount[r] == 2) pairs.push_back(r);
    }

    if (pairs.size() >= 2) {
        int kicker = -1;
        for (int r = 14; r >= 2; r--) {
            if (rankCount[r] == 1) {
                kicker = r;
                break;
            }
        }
        return HandValue{ 2, {pairs[0], pairs[1], kicker} };
    }

    if (pairs.size() == 1) {
        std::vector<int> result = { pairs[0] };
        for (int r = 14; r >= 2; r--) {
            if (rankCount[r] == 1) result.push_back(r);
        }
        return HandValue{ 1, result };
    }

    return HandValue{ 0, ranks };
}

bool betterHand(const HandValue& a, const HandValue& b) {
    if (a.category != b.category) {
        return a.category > b.category;
    }

    return a.tiebreakers > b.tiebreakers;
}

bool equalHand(const HandValue& a, const HandValue& b) {
    return a.category == b.category && a.tiebreakers == b.tiebreakers;
}

HandValue evaluate7Cards(const std::vector<Card>& cards) {
    HandValue best{ -1, {} };

    for (int a = 0; a < 7; a++) {
        for (int b = a + 1; b < 7; b++) {
            std::vector<Card> five;

            for (int i = 0; i < 7; i++) {
                if (i != a && i != b) {
                    five.push_back(cards[i]);
                }
            }

            HandValue current = evaluate5Cards(five);

            if (best.category == -1 || betterHand(current, best)) {
                best = current;
            }
        }
    }

    return best;
}

SimulationResult runWorker(
    const std::vector<Card>& p1,
    const std::vector<Card>& p2,
    const std::vector<Card>& baseDeck,
    long long simulations,
    int threadId
) {
    SimulationResult result;

    std::mt19937 rng(
        std::random_device{}() + threadId * 1009
    );

    for (long long i = 0; i < simulations; i++) {
        std::vector<Card> deck = baseDeck;
        std::shuffle(deck.begin(), deck.end(), rng);

        std::vector<Card> board(deck.begin(), deck.begin() + 5);

        std::vector<Card> p1Seven = p1;
        std::vector<Card> p2Seven = p2;

        for (Card c : board) {
            p1Seven.push_back(c);
            p2Seven.push_back(c);
        }

        HandValue h1 = evaluate7Cards(p1Seven);
        HandValue h2 = evaluate7Cards(p2Seven);

        if (betterHand(h1, h2)) result.p1Wins++;
        else if (betterHand(h2, h1)) result.p2Wins++;
        else result.ties++;
    }

    return result;
}

SimulationResult runParallelMonteCarlo(
    const std::vector<Card>& p1,
    const std::vector<Card>& p2,
    long long simulations,
    int numThreads
) {
    std::vector<Card> baseDeck = createDeck();

    for (Card c : p1) removeCardFromDeck(baseDeck, c);
    for (Card c : p2) removeCardFromDeck(baseDeck, c);

    std::vector<std::thread> threads;
    std::vector<SimulationResult> partialResults(numThreads);

    long long simsPerThread = simulations / numThreads;
    long long remainder = simulations % numThreads;

    for (int t = 0; t < numThreads; t++) {
        long long simsForThread = simsPerThread + (t < remainder ? 1 : 0);

        threads.emplace_back([&, t, simsForThread]() {
            partialResults[t] = runWorker(p1, p2, baseDeck, simsForThread, t);
            });
    }

    for (std::thread& th : threads) {
        th.join();
    }

    SimulationResult finalResult;

    for (const SimulationResult& r : partialResults) {
        finalResult.p1Wins += r.p1Wins;
        finalResult.p2Wins += r.p2Wins;
        finalResult.ties += r.ties;
    }

    return finalResult;
}

bool hasDuplicates(const std::vector<Card>& cards) {
    for (int i = 0; i < (int)cards.size(); i++) {
        for (int j = i + 1; j < (int)cards.size(); j++) {
            if (sameCard(cards[i], cards[j])) return true;
        }
    }
    return false;
}

int main() {
    std::string p1c1, p1c2, p2c1, p2c2;
    long long simulations;
    int numThreads;

    std::cout << "========================================\n";
    std::cout << " Parallelized Preflop Monte Carlo Simulator\n";
    std::cout << "========================================\n\n";

    std::cout << "Enter Player 1 card 1, e.g. Ah: ";
    std::cin >> p1c1;

    std::cout << "Enter Player 1 card 2, e.g. Kh: ";
    std::cin >> p1c2;

    std::cout << "Enter Player 2 card 1, e.g. Qs: ";
    std::cin >> p2c1;

    std::cout << "Enter Player 2 card 2, e.g. Qd: ";
    std::cin >> p2c2;

    std::cout << "Enter number of simulations, e.g. 1000: ";
    std::cin >> simulations;

    std::cout << "Enter number of threads, e.g. 8: ";
    std::cin >> numThreads;

    Card c1 = stringToCard(p1c1);
    Card c2 = stringToCard(p1c2);
    Card c3 = stringToCard(p2c1);
    Card c4 = stringToCard(p2c2);

    std::vector<Card> allCards = { c1, c2, c3, c4 };

    for (Card c : allCards) {
        if (c.rank == -1) {
            std::cout << "Invalid card input.\n";
            return 1;
        }
    }

    if (hasDuplicates(allCards)) {
        std::cout << "Duplicate cards are not allowed.\n";
        return 1;
    }

    if (simulations <= 0 || numThreads <= 0) {
        std::cout << "Simulations and threads must be positive.\n";
        return 1;
    }

    std::vector<Card> p1 = { c1, c2 };
    std::vector<Card> p2 = { c3, c4 };

    std::cout << "\nPlayer 1: " << cardToString(c1) << " " << cardToString(c2) << "\n";
    std::cout << "Player 2: " << cardToString(c3) << " " << cardToString(c4) << "\n";
    std::cout << "Simulations: " << simulations << "\n";
    std::cout << "Threads: " << numThreads << "\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    SimulationResult result = runParallelMonteCarlo(p1, p2, simulations, numThreads);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> runtime = end - start;

    double p1Equity = (result.p1Wins + 0.5 * result.ties) / simulations;
    double p2Equity = (result.p2Wins + 0.5 * result.ties) / simulations;

    double standardError = std::sqrt((p1Equity * (1.0 - p1Equity)) / simulations);
    double confidenceInterval = 1.96 * standardError;

    double simsPerSecond = simulations / runtime.count();

    std::cout << "Results:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "P1 wins: " << result.p1Wins << "\n";
    std::cout << "P2 wins: " << result.p2Wins << "\n";
    std::cout << "Ties:    " << result.ties << "\n\n";

    std::cout << "P1 equity: " << p1Equity * 100.0 << "%";
    std::cout << " +/- " << confidenceInterval * 100.0 << "%\n";

    std::cout << "P2 equity: " << p2Equity * 100.0 << "%";
    std::cout << " +/- " << confidenceInterval * 100.0 << "%\n\n";

    std::cout << "Performance:\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Runtime: " << runtime.count() << " seconds\n";
    std::cout << "Throughput: " << simsPerSecond << " simulations/sec\n";

    return 0;
}