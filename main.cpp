#include <bits/stdc++.h>
using namespace std;

// Include our game implementation header
#include "game.h"

static void loadCards(Pile& draw_pile) {
    int n;
    if (!(cin >> n)) return;
    for (int i = 0; i < n; ++i) {
        string card_type; cin >> card_type;
        Card* card = nullptr;
        if (card_type == "SCORE") {
            int point; cin >> point; card = new ScoreCard(point);
        } else if (card_type == "SKILL") {
            string skill_type; cin >> skill_type;
            if (skill_type == "Multiplier") card = new SkillCard(SkillCardType::MULTIPLIER);
            else if (skill_type == "HeadButt") card = new SkillCard(SkillCardType::HEAD_BUTT);
        } else if (card_type == "POWER") {
            int plus_count; cin >> plus_count; card = new PowerCard(plus_count);
        }
        if (!card) throw runtime_error("Invalid Operation");
        draw_pile.appendCard(card);
    }
}

static void processCommands(GameController& game) {
    string command;
    while (cin >> command) {
        if (command == "draw") {
            game.draw();
        } else if (command == "play") {
            int idx; cin >> idx; game.play(idx);
        } else if (command == "finish") {
            game.finish();
        } else if (command == "exit") {
            break;
        } else if (command == "query") {
            string t; cin >> t;
            if (t == "score") cout << game.queryScore() << '\n';
            else if (t == "hand") cout << game.queryHandSize() << '\n';
            else if (t == "draw_pile") cout << game.queryDrawPileSize() << '\n';
            else if (t == "discard_pile") cout << game.queryDiscardPileSize() << '\n';
            else throw runtime_error("Invalid Operation");
        } else {
            throw runtime_error("Invalid Operation");
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int shuffle_mode; if (!(cin >> shuffle_mode)) return 0;
    GameController game(shuffle_mode);
    loadCards(game.drawPile());
    processCommands(game);
    return 0;
}

