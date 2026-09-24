#include "Automata.h"
#include <functional>
#include <utility>

enum : int { Split = 256, Match = 257 };

State* post2nfa(char* postfix, State* matchState);


void exprConcat(Expression e2, Expression e1, std::function<void(Expression s)> push);
void exprUnion(Expression e2, Expression e1, std::function<void(Expression s)> push);
void exprQuestion(Expression e, std::function<void(Expression s)> push);
void exprKleene(Expression e, std::function<void(Expression s)> push);
void exprKleenePositive(Expression e, std::function<void(Expression s)> push);
void exprFromSymbol(char symbol, std::function<void(Expression s)> push);


Ptrlist newList(State** outPtr);
Ptrlist append(Ptrlist l1, const Ptrlist& l2);
void patch(const Ptrlist& l, State* s);


List& startList(State* start, List& l, int& listId);
void step(List& currentStates, int c, List& nextStates, int& listId);
void addState(List& l, State* s, int listId);
bool isMatch(const List& l, const State* matchState);


Automata::Automata(char* postfix) {
    m_MatchState = { .symbol = Match, .out1 = nullptr, .out2 = nullptr, .lastList = 0 };
    m_StartState = post2nfa(postfix, &m_MatchState);
}

bool Automata::match(char* s) {
    List* currentStates = &startList(m_StartState, m_List1, m_ListId);
    List* nextStates = &m_List2;
    while(*s != '\0') {
        step(*currentStates, *s, *nextStates, m_ListId);
        std::swap(currentStates, nextStates);
        s++;
    }
    return isMatch(*currentStates, &m_MatchState);
}

State* post2nfa(char* postfix, State* matchState) {
    Expression stack[1000];
    Expression* stackPtr = stack;

    auto push = [&stackPtr](Expression s){
        *stackPtr = s;
        stackPtr++;
    };
    auto pop = [&stackPtr](){
        stackPtr--;
        Expression top = *stackPtr;
        return top;
    };

    while (*postfix) {
        switch (*postfix) {
            case '.':
                exprConcat(pop(), pop(), push);
                break;
            case '|':
                exprUnion(pop(), pop(), push);
                break;
            case '?':
                exprQuestion(pop(), push);
                break;
            case '*':
                exprKleene(pop(), push);
                break;
            case '+':
                exprKleenePositive(pop(), push);
                break;
            default:
                exprFromSymbol(*postfix, push);
                break;
        }
        postfix++;
    }

    Expression e = pop();
    patch(e.out, matchState);
    return e.start;
}



void exprConcat(Expression e2, Expression e1, std::function<void(Expression s)> push) {
    patch(e1.out, e2.start);
    push(Expression(e1.start, e2.out));
}

void exprUnion(Expression e2, Expression e1, std::function<void(Expression s)> push) {
    State* s = new State(Split, e1.start, e2.start);
    push(Expression(s, append(e1.out, e2.out)));
}

void exprQuestion(Expression e, std::function<void(Expression s)> push) {
    State* s = new State(Split, e.start, nullptr);
    push(Expression(s, append(e.out, newList(&s->out2))));
}

void exprKleene(Expression e, std::function<void(Expression s)> push) {
    State* s = new State(Split, e.start, nullptr);
    patch(e.out, s);
    push(Expression(s, newList(&s->out2)));
}

void exprKleenePositive(Expression e, std::function<void(Expression s)> push) {
    State* s = new State(Split, e.start, nullptr);
    patch(e.out, s);
    push(Expression(e.start, newList(&s->out2)));
}

void exprFromSymbol(char symbol, std::function<void(Expression s)> push) {
    State* s = new State(symbol, nullptr, nullptr);
    push(Expression(s, newList(&s->out1)));
}


Ptrlist newList(State** outPtr) {
    return { outPtr };
}

Ptrlist append(Ptrlist l1, const Ptrlist& l2) {
    l1.insert(l1.end(), l2.begin(), l2.end());
    return l1;
}

void patch(const Ptrlist& l, State* s) {
    for (State** slot : l)
        *slot = s;
}


// Sets currentStates to the epsilon-closure of start: start itself, plus every state
// reachable from it through Split states without consuming input.
List& startList(State* start, List& l, int& listId) {
    listId++;
    l.size = 0;
    addState(l, start, listId);
    return l;
}

// Advances every state in currentStates that matches c into nextStates.
void step(List& currentStates, int c, List& nextStates, int& listId) {
    listId++;
    nextStates.size = 0;
    for (int i = 0; i < currentStates.size; i++) {
        State* s = currentStates.states[i];
        if (s->symbol == c)
            addState(nextStates, s->out1, listId);
    }
}

// Adds s to l, following Split states' epsilon arrows instead of adding the
// Split state itself. listId dedupes states already added during this step.
void addState(List& l, State* s, int listId) {
    if (s == nullptr || s->lastList == listId)
        return;
    s->lastList = listId;
    if (s->symbol == Split) {
        addState(l, s->out1, listId);
        addState(l, s->out2, listId);
        return;
    }
    l.states[l.size++] = s;
}

bool isMatch(const List& l, const State* matchState) {
    for (int i = 0; i < l.size; i++) {
        if (l.states[i] == matchState)
            return true;
    }
    return false;
}
