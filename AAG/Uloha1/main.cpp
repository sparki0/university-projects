#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>

#include <cassert>
using namespace std;

using State = unsigned int;
using Symbol = char;

struct MISNFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, std::set<State>> m_Transitions;
    std::set<State> m_InitialStates;
    std::set<State> m_FinalStates;
};

struct DFA {
    std::set<State> m_States;
    std::set<Symbol> m_Alphabet;
    std::map<std::pair<State, Symbol>, State> m_Transitions;
    State m_InitialState;
    std::set<State> m_FinalStates;

    bool operator==(const DFA& dfa)
    {
        return std::tie(m_States, m_Alphabet, m_Transitions, m_InitialState, m_FinalStates) == std::tie(dfa.m_States, dfa.m_Alphabet, dfa.m_Transitions, dfa.m_InitialState, dfa.m_FinalStates);
    }
};

#endif

map<State ,map<Symbol,set<State>>> getStates(const MISNFA& nfa) {
    set<State> achievableStates;
    set<State> usefulState;
    vector<map<State,set<Symbol>>> graph(nfa.m_States.size());

    vector<bool> visited(nfa.m_InitialStates.size(), false);

    for(auto & i : nfa.m_Transitions) {
        for(auto & j : i.second) {
            graph[i.first.first][j].emplace(i.first.second);
        }
    }

    queue<State> q;
    for(auto & i : nfa.m_InitialStates) {
        q.emplace(i);
        achievableStates.emplace(i);
        visited[i] = true;
    }

    vector<set<State>> parents(nfa.m_States.size());
    while( !q.empty() ){
        State current = q.front();
        q.pop();
        for(auto & i : graph[current]) {
            parents[i.first].emplace(current);
            if( ! visited[i.first] ){
                achievableStates.emplace(i.first);
                visited[i.first] = true;
                q.push(i.first);
            }
        }
    }

    visited.assign(nfa.m_States.size(),false);
    for(auto & i : nfa.m_FinalStates) {
        visited[i] = true;
        q.push(i);
        usefulState.emplace(i);
    }

    while( ! q.empty() ) {
        State current = q.front();
        q.pop();
        for(auto & i : parents[current]) {
            usefulState.emplace(i);
            if( ! visited[i] ) {
                visited[i] = true;
                q.push(i);
            }
        }
    }

    set<State> resultStates;

    for(auto & i: usefulState) {
        if (achievableStates.find(i) != achievableStates.end()) {
            resultStates.emplace(i);
        }
    }


    map<State ,map<Symbol,set<State>>> result;
    for(auto & i : resultStates) {
        for(auto & j : graph[i]) {
            if(resultStates.find(j.first) != resultStates.end()) {
                for(auto & x: j.second){
                    result[i][x].emplace(j.first);
                }
            }
        }
    }

    return result;
}

DFA determinize(const MISNFA& nfa){
    map<State ,map<Symbol,set<State>>> usefulStates = getStates(nfa);
    set<State> startStates;
    for(auto & i : nfa.m_InitialStates) {
            startStates.emplace(i);

    }
    map<set<State>, State> interStates;
    map< pair<set<State> , Symbol> , set<State>> mapa;
    interStates.emplace(startStates,State(-1));
    queue<set<State>> q;
    q.push(startStates);
    while (!q.empty()) {
        auto current = q.front();
        q.pop();
        for(auto & c : nfa.m_Alphabet) {
            set<State> newInterState;
            for(auto & i: current) {
                for(auto & dest: usefulStates[i][c]) {
                    newInterState.emplace(dest);
                }
            }
            if( !newInterState.empty() ) {
                mapa[make_pair(current,c)] = newInterState;
                if(  interStates.emplace(newInterState,State(-1)).second ) {
                    q.push(newInterState);
                }
            }


        }
    }


    DFA result;
    result.m_Alphabet = nfa.m_Alphabet;

    interStates[startStates] = 0;
    result.m_InitialState = 0;
    State index = 1;
    for(auto & i : interStates) {
        if(i.second == State(-1)) {
            i.second = index++;
        }
        result.m_States.emplace(i.second);
    }


    for(auto & i : interStates) {
        for(auto & x: nfa.m_FinalStates) {
            if(i.first.find(x) != i.first.end()) {
                result.m_FinalStates.emplace(i.second);
                continue;
            }
        }
    }

    for(auto & i : interStates) {
        for(auto & chars : result.m_Alphabet) {
            auto it = mapa.find(make_pair(i.first,chars));
            if(it != mapa.end()) {
                State from = i.second;
                State to = interStates[it->second];
                result.m_Transitions.emplace(make_pair(from,chars),to);
            }
        }
    }






    return result;
}

#ifndef __PROGTEST__


int main()
{
  


    return 0;
}
#endif
