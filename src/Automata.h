#pragma once

#include <vector>

struct State {
    int symbol;
    State* out1;
    State* out2;
    int lastList;
};

// Ptrlist: a set of addresses of dangling (not-yet-connected) out/out1 fields.
// This is NOT a chain of real State transitions - it's bookkeeping for holes
// that haven't been filled in yet. list1 opens one hole; append merges two
// hole-sets into one (a union of dangling arrows - parallel, not sequential);
// patch later fills every hole in the set with the same successor state at once.
using Ptrlist = std::vector<State**>;

struct Expression {
    State* start;
    Ptrlist out;
};

// List: the set of NFA states "currently active" during simulation (used by
// match/step/addState). Unlike Ptrlist, this is a plain array of live State
// pointers - a different structure for a different job, even though your
// original code used one struct for both.
struct List {
    State** states;
    int size;
};

class Automata {
public:
    explicit Automata(char* postfix);
    Automata(const Automata&) = delete;
    Automata& operator=(const Automata&) = delete;
    Automata(Automata&&) = delete;
    Automata& operator=(Automata&&) = delete;

    bool match(char* s);

private:
    static constexpr int kMaxStates = 1000;

    State* m_StartState = nullptr;
    State m_MatchState{};
    int m_ListId = 0;

    State* m_List1Buffer[kMaxStates];
    State* m_List2Buffer[kMaxStates];
    List m_List1{ m_List1Buffer, 0 };
    List m_List2{ m_List2Buffer, 0 };
};
