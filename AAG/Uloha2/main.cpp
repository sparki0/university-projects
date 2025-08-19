#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

using namespace std;
using Symbol = char;
using Word = std::vector<Symbol>;

struct Grammar {
    std::set<Symbol> m_Nonterminals;
    std::set<Symbol> m_Terminals;
    std::vector<std::pair<Symbol, std::vector<Symbol>>> m_Rules;
    Symbol m_InitialSymbol;
};
#endif


map<pair<Symbol,Symbol>, vector<size_t>> product(map<Symbol,vector<size_t>>& left, map<Symbol,vector<size_t>>& right) {
    map<pair<Symbol,Symbol>, vector<size_t>> result;
    for(auto & x: left) {
        for (auto & y: right) {
            auto pair = make_pair(x.first,y.first);
            vector<size_t> vec = x.second;
            vec.insert(vec.end(),y.second.begin(),y.second.end());
            result[pair]  = std::move(vec);
        }
    }
    return result;
}

map<Symbol, vector<size_t>> getStates(const Grammar& grammar, map<pair<Symbol,Symbol>, vector<size_t>>& pairs) {
    map<Symbol, vector<size_t>> res;
    for(size_t i = 0; i < grammar.m_Rules.size(); ++i) {
        for(auto & v : pairs) {
            vector<Symbol> temp = {v.first.first,v.first.second};
            if(grammar.m_Rules[i].second == temp) {
                res[grammar.m_Rules[i].first].clear();
                res[grammar.m_Rules[i].first].push_back(i);
                res[grammar.m_Rules[i].first].insert(res[grammar.m_Rules[i].first].end(), v.second.begin(),v.second.end());
            }
        }
    }
    return res;
}

std::vector<size_t> CYK(const Grammar& grammar, const Word& word) {
    if(word.empty()) {
        for(size_t i = 0; i < grammar.m_Rules.size(); ++i) {
            if(grammar.m_Rules[i].second.empty()) {
                return {i};
            }
        }
        return {};
    }

    vector<vector<map<Symbol,vector<size_t>>>> array(word.size(), vector<map<Symbol,vector<size_t>>>(word.size()));
    for(size_t i = 0; i < word.size(); ++i) {
        for(size_t j = 0; j < grammar.m_Rules.size(); ++j) {
            for(auto& k : grammar.m_Rules[j].second) {
                if(k == word[i]){
                    array[i][0][grammar.m_Rules[j].first].emplace_back(j);
                    break;
                }
            }
        }
    }

    size_t colLen = array.size() - 1;

    for(size_t col = 1; col < array.size(); ++col) { // колонка
        for(size_t i = 0; i < colLen; ++i) {    // строка
            for(size_t j = 0, m = i + 1, n = col - 1; j < col; ++j, m++, n--) {// правая и левая
                auto pairs = product(array[i][j],array[m][n]);
                auto res = getStates(grammar, pairs);
                for(auto & x: res) {
                    array[i][col][x.first].clear();
                    array[i][col][x.first] = std::move(x.second);
                }
            }
        }
        colLen--;
    }
    vector<size_t> res;
    auto it = array[0][word.size() - 1].find(grammar.m_InitialSymbol);
    if(it != array[0][word.size() - 1].end()) {
        res = it->second;
    }
    return res;
}

std::vector<size_t> trace(const Grammar& grammar, const Word& word) {
    return CYK(grammar,word);
}

#ifndef __PROGTEST__
int main()
{
    Grammar g0{
            {'A', 'B', 'C', 'S'},
            {'a', 'b'},
            {
                    {'S', {'A', 'B'}},
                    {'S', {'B', 'C'}},
                    {'A', {'B', 'A'}},
                    {'A', {'a'}},
                    {'B', {'C', 'C'}},
                    {'B', {'b'}},
                    {'C', {'A', 'B'}},
                    {'C', {'a'}},
            },
            'S'};

    assert(trace(g0, {'b', 'a', 'a', 'b', 'a'}) == std::vector<size_t>({0, 2, 5, 3, 4, 6, 3, 5, 7}));
    assert(trace(g0, {'b'}) == std::vector<size_t>({}));
    assert(trace(g0, {'a'}) == std::vector<size_t>({}));
    assert(trace(g0, {}) == std::vector<size_t>({}));
    assert(trace(g0, {'a', 'a', 'a', 'a', 'a'}) == std::vector<size_t>({1, 4, 6, 3, 4, 7, 7, 7, 7}));
    assert(trace(g0, {'a', 'b'}) == std::vector<size_t>({0, 3, 5}));
    assert(trace(g0, {'b', 'a'}) == std::vector<size_t>({1, 5, 7}));
    assert(trace(g0, {'c', 'a'}) == std::vector<size_t>({}));

    Grammar g1{
            {'A', 'B'},
            {'x', 'y'},
            {
                    {'A', {}},
                    {'A', {'x'}},
                    {'B', {'x'}},
                    {'A', {'B', 'B'}},
                    {'B', {'B', 'B'}},
            },
            'A'};
    assert(trace(g1, {}) == std::vector<size_t>({0}));
    assert(trace(g1, {'x'}) == std::vector<size_t>({1}));
    assert(trace(g1, {'x', 'x'}) == std::vector<size_t>({3, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 2, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 4, 2, 2, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2}));
    assert(trace(g1, {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'}) == std::vector<size_t>({3, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2}));

    Grammar g2{
            {'A', 'B'},
            {'x', 'y'},
            {
                    {'A', {'x'}},
                    {'B', {'x'}},
                    {'A', {'B', 'B'}},
                    {'B', {'B', 'B'}},
            },
            'B'};

    assert(trace(g2, {}) == std::vector<size_t>({}));
    assert(trace(g2, {'x'}) == std::vector<size_t>({1}));
    assert(trace(g2, {'x', 'x'}) == std::vector<size_t>({3, 1, 1}));
    assert(trace(g2, {'x', 'x', 'x'}) == std::vector<size_t>({3, 3, 1, 1, 1}));

    Grammar g3{
            {'A', 'B', 'C', 'D', 'E', 'S'},
            {'a', 'b'},
            {
                    {'S', {'A', 'B'}},
                    {'S', {'S', 'S'}},
                    {'S', {'a'}},
                    {'A', {'B', 'S'}},
                    {'A', {'C', 'D'}},
                    {'A', {'b'}},
                    {'B', {'D', 'D'}},
                    {'B', {'b'}},
                    {'C', {'D', 'E'}},
                    {'C', {'b'}},
                    {'C', {'a'}},
                    {'D', {'a'}},
                    {'E', {'S', 'S'}},
            },
            'S'};

    assert(trace(g3, {}) == std::vector<size_t>({}));
    assert(trace(g3, {'b'}) == std::vector<size_t>({}));
    assert(trace(g3, {'a', 'b', 'a', 'a', 'b'}) == std::vector<size_t>({1, 2, 0, 3, 7, 1, 2, 2, 7}));
    assert(trace(g3, {'a', 'b', 'a', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'a'}) == std::vector<size_t>({1, 1, 0, 4, 8, 11, 12, 0, 5, 6, 11, 11, 0, 4, 9, 11, 7, 11, 7, 2, 2}));
}
#endif
