#ifndef __PROGTEST__
#include <cassert>
#include <cstdarg>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <array>

// Note that std::pop_heap and std::push_heap are disabled
#include <algorithm>

#include <vector>
#include <deque>
#include <random>
#include <type_traits>


struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
  va_list args1;
  va_list args2;
  va_start(args1, f);
  va_copy(args2, args1);

  std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
  va_end(args1);

  vsnprintf(buf.data(), buf.size() + 1, f, args2);
  va_end(args2);

  return buf;
}

// We use std::multiset as a reference to check our implementation.
// It is not available in progtest :)
#include <set>

// On the other hand, Link (as seen in the harder version of this task)
// is still available in progtest.

#endif


// TODO implement
template < typename T, typename Comp = std::less<T> >
struct BinaryHeap {
    BinaryHeap() = default;

    explicit BinaryHeap(Comp comp) : _comparator(comp)
    {
    }

    bool empty() const {
        return _tree.empty();
    }

    size_t size() const {
      return _tree.size();
    }

    const T& min() const {
        if(_tree.empty()) {
            throw std::out_of_range("Heap is empty");
        }
        return _tree[0];
    }

    T extract_min() {
        if(_tree.empty()) {
            throw std::out_of_range("Heap is empty");
        }
        std::swap(_tree[0], _tree.back());
        T res = std::move(_tree.back());
        _tree.pop_back();
        bubbleDown();
        return res;
    }

    void bubbleDown() {
        for(size_t i = 0; i < _tree.size() || i != _tree.size(); ) {
            size_t left = 2 * i + 1;
            size_t right = left + 1;
            size_t choosenOne;
            if(left >= _tree.size()) {
                return;
            }
            else {
                choosenOne = _comparator(_tree[left],_tree[right]) ? left : right;
            }

            if( _comparator(_tree[i], _tree[choosenOne]) ){
                return ;
            }
            std::swap(_tree[i],_tree[choosenOne]);
            i = choosenOne ;
        }
    }

    void push(T val) {
        _tree.emplace_back(std::move(val));
        bubbleUp();
    }

    void bubbleUp() {
        for(size_t i = _tree.size() - 1; i != 0 ;) {
            size_t batya = (i - 1) / 2;
            if( _comparator(_tree[batya], _tree[i]) ) {
                return;
            }
            std::swap(_tree[batya], _tree[i]);
            i = batya;
        }
    }

    std::vector<T> _tree;
    Comp _comparator;
  // Helpers to enable testing.
  struct TestHelper {
    static const T& index_to_value(const BinaryHeap& H, size_t index) {
        return H._tree[index];
    }

    static size_t root_index() { return 0; }
    static size_t parent(size_t index) {
        return (index - 1)/ 2 ;
    }
    static size_t child(size_t parent, size_t ith) {
        return (parent * 2 + 1) +ith;
    }
  };
};


#ifndef __PROGTEST__

#define CHECK(cond, ...) do { \
    if (!(cond)) throw Error(fmt(__VA_ARGS__)); \
  } while (0)

template <
  typename T, typename Cmp,
  template < typename, typename > typename Tested_
>
class HeapTester {
  struct Node;
  using TElem = const Node*;

  struct CmpInternal {
    const HeapTester *t;

    bool operator () (const Node& a, const Node& b) const { return t->_cmp(a.value, b.value); }
    bool operator () (const TElem& a, const TElem& b) const {
      return t->_cmp(a->value, b->value);
    }
  };

  using It = typename std::multiset<Node, CmpInternal>::const_iterator;

  struct Node  {
    T value;
    mutable It rref;

    explicit Node(T value) : value(std::move(value)) {}
    friend class HeapTester;
  };

  public:
  struct Error : std::runtime_error {
    using std::runtime_error::runtime_error;
  };


  explicit HeapTester(Cmp cmp = {})
    : _cmp(std::move(cmp)), _ref(CmpInternal{this}), _t(CmpInternal{this}) {}


  bool empty() const {
    CHECK(_t.empty() == _ref.empty(),
      "_t.empty() == %d, _ref.empty() == %d", _t.empty(), _ref.empty());
    return _t.empty();
  }

  size_t size() const {
    CHECK(_t.size() == _ref.size(),
      "_t.size() == %zu, _ref.size() == %zu", _t.size(), _ref.size());
    return _t.size();
  }


  void push(T val) {
    It it = _ref.insert(Node(std::move(val)));
    it->rref = it;
    _t.push(&*it);
  }

  const T& min() const {
    const Node *n = _min();
    _check_node(n);
    return n->value;
  }

  T extract_min() {
    const Node *r = _min();
    _check_node(r);
    const Node *t = _t.extract_min();
    CHECK(r == t, "_t.min() != _t.extract_min()");
    return std::move(_ref.extract(r->rref).value().value);
  }

  void check_structure() {
    using Helper = typename decltype(_t)::TestHelper;

    auto node_at_index = [&](size_t i) {
      return Helper::index_to_value(_t, i);
    };

    std::vector<const Node*> seen;
    const size_t root = Helper::root_index();
    const size_t max = root + size();
    for (size_t i = root; i < max; i++) {
      const Node *n = node_at_index(i);
      _check_node(n);
      seen.push_back(n);

      if (i != root)
        CHECK(!_cmp(n->value, node_at_index(Helper::parent(i))->value), "parent > son");

      CHECK(Helper::parent(Helper::child(i, 0)) == i, "n->left->parent != n");
      CHECK(Helper::parent(Helper::child(i, 1)) == i, "n->right->parent != n");
    }

    std::sort(seen.begin(), seen.end());
    CHECK(std::unique(seen.begin(), seen.end()) == seen.end(),
      "Some node was seen multiple times");
  }

  private:
  void _check_node(const Node* n) const {
    CHECK(&*n->rref == n, "Node mismatch (rref)");
  }

  const Node* _min() const {
    const Node& rmin = *_ref.begin();
    const Node* tmin = _t.min();
    CHECK(!_cmp(rmin.value, tmin->value), "_ref.min() < _t.min()");
    CHECK(!_cmp(tmin->value, rmin.value), "_t.min() < _ref.min()");
    return tmin;
  }

  Cmp _cmp;
  std::multiset<Node, CmpInternal> _ref;
  Tested_<TElem, CmpInternal> _t;
};

#undef CHECK

template < typename Heap >
void run_test(int max, bool check_structure = false) {
  Heap H;

  auto check = [&]() {
    if (check_structure) H.check_structure();
  };

  for (int i = 0; i < max; i++) {
    H.push((i*991) % (5*max));
    check();
  }

  for (int i = 0; i < 2*max; i++) {
    H.empty();
    H.extract_min();
    check();
    H.push((i*991) % (7*max));
    check();
  }
}

template < typename T, typename Cmp = std::less<T> >
using Tester = HeapTester<T, Cmp, BinaryHeap>;

int main() {
  {
    BinaryHeap<int> H;

    try {
      H.min();
      throw TestFailed("min on empty heap did not throw");
    } catch (const std::out_of_range&) {}

    try {
      H.extract_min();
      throw TestFailed("extract_min on empty heap did not throw");
    } catch (const std::out_of_range&) {}
  }

  try {
    std::cout << "Small test..." << std::endl;
//    run_test<Tester<int>>(20);
    run_test<Tester<int>>(20, true);
//    run_test<Tester<int>>(1'000, true);


    std::cout << "Big test..." << std::endl;
    run_test<Tester<int>>(500'000);

    std::cout << "All tests passed." << std::endl;
  } catch (const TestFailed&) {}
}

#endif


