#ifndef __PROGTEST__
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum Point : size_t {};

struct Path {
    Point from, to;
    unsigned length;

    Path(size_t f, size_t t, unsigned l) : from{f}, to{t}, length{l} {}

    friend bool operator == (const Path& a, const Path& b) {
        return std::tie(a.from, a.to, a.length) == std::tie(b.from, b.to, b.length);
    }

    friend bool operator != (const Path& a, const Path& b) { return !(a == b); }
};

#endif


class Graph {
public:
    Graph(size_t points ,const std::vector<Path>& all_paths): _points(points) {
        _srcCount = 0;
        _paths.resize(_points);
        std::vector<size_t> degCounts(_points, 0);

        for(auto &x : all_paths) {
            degCounts[x.to]++;
            _paths[x.from].emplace_back(x);
        }
        std::queue<size_t> q;

        for( size_t i = 0 ; i < degCounts.size() ; ++i ) {
            if( degCounts[i] == 0 ) {
                q.push(i);
                _srcCount++;
            }
        }
        while( ! q.empty() ) {
            auto current = q.front();
            q.pop();
            _topsorted.push_back(current);
            for( auto & x: _paths[current]) {
                degCounts[x.to]--;
                if( ! degCounts[x.to] ) {
                    q.push(x.to);
                }
            }
        }
    }

    std::vector<Path> work() {
        std::vector<size_t> parent(_points, size_t(-1));
        std::vector<size_t> dist(_points,0);

        for(unsigned long long j : _topsorted) {
            for(auto & x : _paths[ j ]) {
                size_t newDest = dist[j] + x.length;
                if(dist[x.to] < newDest) {
                    dist[x.to] = newDest;
                    parent[x.to] = j;
                }
            }
        }

        size_t maxInd = 0;
        size_t maxLen = dist[maxInd];
        for(size_t j = 1; j < dist.size(); ++j) {
            if(maxLen < dist[j]) {
                maxLen = dist[j];
                maxInd = j;
            }
        }


        std::vector<Path> maxPath;
        for(size_t i = maxInd; parent[i] != size_t(-1); ) {
            maxPath.emplace_back(parent[i],i, maxLen - dist[parent[i]]);
            maxLen = dist[parent[i]];
            i = parent[i];

        }
        std::reverse(maxPath.begin(),maxPath.end());

        return maxPath;
    }

private:
    size_t _srcCount;
    std::vector<size_t> _topsorted;
    std::vector<size_t> _sources;
    size_t _points;
    std::vector<std::vector<Path>> _paths;
};

std::vector<Path> longest_track(size_t points, const std::vector<Path>& all_paths) {
    Graph g(points,all_paths);
    return g.work();
}


#ifndef __PROGTEST__


struct Test {
    unsigned longest_track;
    size_t points;
    std::vector<Path> all_paths;
};

inline const Test TESTS[] = {
        {13, 5, { {3,2,10}, {3,0,9}, {0,2,3}, {2,4,1}} },
        {11, 5, { {3,2,10}, {3,1,4}, {1,2,3}, {2,4,1} } },
        {16, 8, { {3,2,10}, {3,1,1}, {1,2,3}, {1,4,15} } }
};

#define CHECK(cond, ...) do { \
    if (cond) break; \
    printf("Fail: " __VA_ARGS__); \
    printf("\n"); \
    return false; \
  } while (0)

bool run_test(const Test& t) {
    auto sol = longest_track(t.points, t.all_paths);

    unsigned length = 0;
    for (auto [ _, __, l ] : sol) length += l;

    CHECK(t.longest_track == length,
          "Wrong length: got %u but expected %u", length, t.longest_track);

    for (size_t i = 0; i < sol.size(); i++) {
        CHECK(std::count(t.all_paths.begin(), t.all_paths.end(), sol[i]),
              "Solution contains non-existent path: %zu -> %zu (%u)",
              sol[i].from, sol[i].to, sol[i].length);

        if (i > 0) CHECK(sol[i].from == sol[i-1].to,
                         "Paths are not consecutive: %zu != %zu", sol[i-1].to, sol[i].from);
    }

    return true;
}
#undef CHECK

int main() {
    int ok = 0, fail = 0;

    for (auto&& t : TESTS) (run_test(t) ? ok : fail)++;

    if (!fail) printf("Passed all %i tests!\n", ok);
    else printf("Failed %u of %u tests.\n", fail, fail + ok);
}

#endif


