#ifndef GAME_H
#define GAME_H

#include <bits/stdc++.h>

struct Node;
struct GameState;
class Card;
class Pile;

enum SkillCardType { MULTIPLIER, HEAD_BUTT };

class Card {
public:
    Card() = default;
    virtual ~Card() = default;
    virtual void play(GameState& game_state) = 0;
};

class ScoreCard: public Card {
public:
    int point_{};
    explicit ScoreCard(int point): point_(point) {}
    void play(GameState &game_state) override;
    ~ScoreCard() override = default;
};

class SkillCard: public Card {
public:
    SkillCardType type_{};
    explicit SkillCard(SkillCardType t): type_(t) {}
    void play(GameState &game_state) override;
    ~SkillCard() override = default;
};

class PowerCard: public Card {
public:
    int plus_{};
    explicit PowerCard(int plus_count): plus_(plus_count) {}
    void play(GameState &game_state) override;
    ~PowerCard() override = default;
};

struct Node {
    Card* card{nullptr};
    Node* next{nullptr};
    Node* prev{nullptr};
    explicit Node(Card* c): card(c) {}
};

class Pile {
private:
    int size_{};
    Node* head_{nullptr};
    Node* tail_{nullptr};
public:
    friend void outShuffle(GameState&);
    friend void inShuffle(GameState&);
    friend void oddEvenShuffle(GameState&);
    friend class GameController;

    Pile(): size_(0), head_(nullptr), tail_(nullptr) {}
    ~Pile();

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void appendCard(Card* card);
    bool move_tail_to_head(class Pile& other);

private:
    Node* pop_head();
    void push_tail_node(Node* n);
    void push_head_node(Node* n);
    void clear_nodes(bool delete_cards);
};

struct GameState {
    Pile hand_{};
    Pile draw_pile_{};
    Pile discard_pile_{};
    long long score{0};
    long long power_plus{0};
    long long multiplier{1};
};

void outShuffle(GameState &game_state);
void inShuffle(GameState &game_state);
void oddEvenShuffle(GameState &game_state);

class GameController {
private:
    GameState game_state_{};
    void (*shuffle_)(GameState&) = nullptr;
public:
    explicit GameController(int mode);
    void draw();
    void play(int card_to_play);
    void finish();
    void shuffle();
    int queryScore();
    int queryDrawPileSize() const { return game_state_.draw_pile_.size(); }
    int queryHandSize() const { return game_state_.hand_.size(); }
    int queryDiscardPileSize() const { return game_state_.discard_pile_.size(); }
    Pile& drawPile() { return game_state_.draw_pile_; }
};

// ================= Implementations ===================

// Pile
Pile::~Pile() { clear_nodes(true); }

void Pile::clear_nodes(bool delete_cards) {
    Node* cur = head_;
    while (cur) {
        Node* nx = cur->next;
        if (delete_cards) delete cur->card;
        delete cur;
        cur = nx;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

void Pile::appendCard(Card* card) {
    Node* n = new Node(card);
    if (!tail_) {
        head_ = tail_ = n;
    } else {
        tail_->next = n;
        n->prev = tail_;
        tail_ = n;
    }
    ++size_;
}

Node* Pile::pop_head() {
    if (!head_) return nullptr;
    Node* n = head_;
    head_ = head_->next;
    if (head_) head_->prev = nullptr; else tail_ = nullptr;
    n->next = n->prev = nullptr;
    --size_;
    return n;
}

void Pile::push_tail_node(Node* n) {
    n->next = nullptr; n->prev = tail_;
    if (!tail_) { head_ = tail_ = n; }
    else { tail_->next = n; tail_ = n; }
    ++size_;
}

void Pile::push_head_node(Node* n) {
    n->prev = nullptr; n->next = head_;
    if (!head_) { head_ = tail_ = n; }
    else { head_->prev = n; head_ = n; }
    ++size_;
}

bool Pile::move_tail_to_head(Pile& other) {
    if (!tail_) return false;
    Node* t = tail_;
    tail_ = t->prev;
    if (tail_) tail_->next = nullptr; else head_ = nullptr;
    --size_;
    t->next = t->prev = nullptr;
    other.push_head_node(t);
    return true;
}


// Cards
void ScoreCard::play(GameState &gs) {
    long long gained = (long long)(point_ + gs.power_plus) * gs.multiplier;
    gs.score += gained;
    gs.multiplier = 1; // reset after scoring
}

void SkillCard::play(GameState &gs) {
    if (type_ == MULTIPLIER) {
        gs.multiplier += 1;
    } else if (type_ == HEAD_BUTT) {
        (void)gs.discard_pile_.move_tail_to_head(gs.draw_pile_);
    }
}

void PowerCard::play(GameState &gs) {
    gs.power_plus += plus_;
}

// Shuffle functions
void outShuffle(GameState& gs) {
    // move discard in order head->tail to draw tail
    Node* cur = gs.discard_pile_.head_;
    while (cur) {
        Node* nx = cur->next;
        cur->next = cur->prev = nullptr;
        gs.draw_pile_.push_tail_node(cur);
        cur = nx;
        gs.discard_pile_.size_--;
    }
    gs.discard_pile_.head_ = gs.discard_pile_.tail_ = nullptr;
}

void inShuffle(GameState& gs) {
    // move discard in reverse: from tail to head
    Node* cur = gs.discard_pile_.tail_;
    while (cur) {
        Node* pv = cur->prev;
        cur->next = cur->prev = nullptr;
        gs.draw_pile_.push_tail_node(cur);
        cur = pv;
        gs.discard_pile_.size_--;
    }
    gs.discard_pile_.head_ = gs.discard_pile_.tail_ = nullptr;
}

void oddEvenShuffle(GameState& gs) {
    // collect odd indices then even indices from discard head->tail
    vector<Node*> odd, even;
    int idx = 1;
    for (Node* cur = gs.discard_pile_.head_; cur; cur = cur->next, ++idx) {
        if (idx % 2) odd.push_back(cur); else even.push_back(cur);
    }
    for (Node* n : odd) { n->next = n->prev = nullptr; gs.draw_pile_.push_tail_node(n); gs.discard_pile_.size_--; }
    for (Node* n : even) { n->next = n->prev = nullptr; gs.draw_pile_.push_tail_node(n); gs.discard_pile_.size_--; }
    gs.discard_pile_.head_ = gs.discard_pile_.tail_ = nullptr;
}

// GameController
GameController::GameController(int mode) {
    if (mode == 1) shuffle_ = &outShuffle;
    else if (mode == 2) shuffle_ = &inShuffle;
    else shuffle_ = &oddEvenShuffle;
}

void GameController::draw() {
    for (int i = 0; i < 5; ++i) {
        if (game_state_.draw_pile_.empty()) {
            if (!game_state_.discard_pile_.empty()) shuffle();
            else break;
        }
        Node* n = game_state_.draw_pile_.pop_head();
        if (!n) break;
        game_state_.hand_.push_tail_node(n);
    }
}

void GameController::play(int card_to_play) {
    if (card_to_play <= 0 || card_to_play > game_state_.hand_.size_) return;
    Node* cur = game_state_.hand_.head_;
    for (int i = 1; i < card_to_play; ++i) cur = cur->next;
    if (cur->prev) cur->prev->next = cur->next; else game_state_.hand_.head_ = cur->next;
    if (cur->next) cur->next->prev = cur->prev; else game_state_.hand_.tail_ = cur->prev;
    game_state_.hand_.size_--;
    Card* c = cur->card;
    c->play(game_state_);
    if (dynamic_cast<PowerCard*>(c) != nullptr) {
        delete c; delete cur;
    } else {
        game_state_.discard_pile_.push_tail_node(cur);
    }
}

void GameController::shuffle() { if (shuffle_) shuffle_(game_state_); }

void GameController::finish() {
    // move all hand cards to discard tail in order
    Node* cur = game_state_.hand_.head_;
    while (cur) {
        Node* nx = cur->next;
        cur->next = cur->prev = nullptr;
        game_state_.discard_pile_.push_tail_node(cur);
        cur = nx;
        game_state_.hand_.size_--;
    }
    game_state_.hand_.head_ = game_state_.hand_.tail_ = nullptr;
}

int GameController::queryScore() { return (int)game_state_.score; }

#endif // GAME_H

