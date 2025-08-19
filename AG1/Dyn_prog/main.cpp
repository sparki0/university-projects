#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>

using Price = unsigned long long;
using Employee = size_t;
inline constexpr Employee NO_EMPLOYEE = -1;
using Gift = size_t;

#endif

size_t getIndBiggerOfTwo(std::vector<std::vector<std::pair<Price,size_t>>>& table, size_t row) {
    return table[row][0] > table[row][1] ? 0 : 1;
}

std::pair<Price,size_t> getMinPair(std::vector<std::vector<std::pair<Price,size_t>>>& table, size_t row, size_t col) {
    std::pair<Price,size_t> minL = table[row][0];
    std::pair<Price,size_t> minR = table[row][1];
    if(minL.second == col) {
        return minR;
    }
    else if(minR.second == col) {
        return minL;
    }

    return minL.first < minR.first ? minL : minR;
}

void fillTable(
        std::vector<std::vector<std::pair<Price,size_t>>>& table,
        std::vector<std::vector<Employee>>& graph,
        const std::vector<Price>& gift_price,
        Employee start
)
{
    std::stack<Employee> stack;
    stack.push(start);
    std::vector<Employee> added(graph.size(), false);
    while( !stack.empty() ) {
        Employee emp = stack.top();
        if(!added[emp]){
            for(auto & w : graph[emp]) {
                stack.push(w);
            }
            added[emp] = true;
            continue;
        }
        stack.pop();

        for(size_t i = 0; i < gift_price.size(); ++i) {
            size_t sum = gift_price[i];
            for(auto & x: graph[emp]) {
                auto pairMin = getMinPair(table,x,i);
                sum += pairMin.first;
            }
            size_t ind = getIndBiggerOfTwo(table,emp);
            if(table[emp][ind].first > sum){
                table[emp][ind] = std::make_pair(sum,i);
            }

        }
    }
}

void fillGifts(
        std::vector<std::vector<std::pair<Price,size_t>>>& table,
        std::vector<std::vector<Employee>>& graph,
        std::pair<Price, std::vector<Gift>>& gifts,
        Employee start
)
{
    std::queue<std::pair<Employee,size_t>> q;
    q.emplace(start, getMinPair(table,start,size_t(-1)).second);
    while( !q.empty() ) {
        auto curr = q.front();
        q.pop();
        gifts.second[curr.first] = curr.second;
        for(auto & x : graph[curr.first]) {
            q.emplace(x, getMinPair(table,x, curr.second).second);
        }
    }
}

std::pair<Price, std::vector<Gift>> optimize_gifts(
        const std::vector<Employee>&    boss,
        const std::vector<Price>&       gift_price
)
{
    std::vector<std::vector<std::pair<Price,size_t>>> table(boss.size(),
                                                            std::vector<std::pair<Price,size_t>>(2,std::make_pair(Price(-1),size_t(-1))));
    std::vector<std::vector<Employee>> graph(boss.size());
    std::vector<Employee> megabosses;
    for(size_t i = 0; i < boss.size(); ++i) {
        if(boss[i] == NO_EMPLOYEE) {
            megabosses.emplace_back(i);
            continue;
        }
        graph[boss[i]].emplace_back(i);
    }
    std::pair<Price, std::vector<Gift>> result = std::make_pair(0,std::vector<Gift>(boss.size()));

    for(Employee mboss : megabosses){
        fillTable(table,graph, gift_price ,mboss);
        fillGifts(table, graph ,result,mboss);
        result.first += getMinPair(table,mboss,size_t(-1)).first;
    }

    return result;
}

#ifndef __PROGTEST__

const std::tuple<Price, std::vector<Employee>, std::vector<Price>> EXAMPLES[] = {
        { 17, {1,2,3,4,NO_EMPLOYEE}, {25,4,18,3} },
        { 16, {4,4,4,4,NO_EMPLOYEE}, {25,4,18,3} },
        { 17, {4,4,3,4,NO_EMPLOYEE}, {25,4,18,3} },
        { 24, {4,4,3,4,NO_EMPLOYEE,3,3}, {25,4,18,3} },
};

#define CHECK(cond, ...) do { \
    if (cond) break; \
    printf("Test failed: " __VA_ARGS__); \
    printf("\n"); \
    return false; \
  } while (0)

bool test(Price p, const std::vector<Employee>& boss, const std::vector<Price>& gp) {
    auto&& [ sol_p, sol_g ] = optimize_gifts(boss, gp);
    CHECK(sol_g.size() == boss.size(),
          "Size of the solution: expected %zu but got %zu.", boss.size(), sol_g.size());

    Price real_p = 0;
    for (Gift g : sol_g) real_p += gp[g];
    CHECK(real_p == sol_p, "Sum of gift prices is %llu but reported price is %llu.", real_p, sol_p);

    if (0) {
        for (Employee e = 0; e < boss.size(); e++) printf(" (%zu)%zu", e, sol_g[e]);
        printf("\n");
    }

    for (Employee e = 0; e < boss.size(); e++)
        CHECK(boss[e] == NO_EMPLOYEE || sol_g[boss[e]] != sol_g[e],
              "Employee %zu and their boss %zu has same gift %zu.", e, boss[e], sol_g[e]);

    CHECK(p == sol_p, "Wrong price: expected %llu got %llu.", p, sol_p);

    return true;
}
#undef CHECK

int main() {
    int ok = 0, fail = 0;
    for (auto&& [ p, b, gp ] : EXAMPLES) (test(p, b, gp) ? ok : fail)++;

    if (!fail) printf("Passed all %d tests!\n", ok);
    else printf("Failed %d of %d tests.", fail, fail + ok);
}

#endif


