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
#include <random>
#include <type_traits>

// We use std::set as a reference to check our implementation.
// It is not available in progtest :)
#include <set>

template < typename T >
struct Ref {
    size_t size() const { return _data.size(); }
    const T* find(const T& value) const {
        auto it = _data.find(value);
        if (it == _data.end()) return nullptr;
        return &*it;
    }
    bool insert(const T& value) { return _data.insert(value).second; }
    bool erase(const T& value) { return _data.erase(value); }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

private:
    std::set<T> _data;
};

#endif


namespace config {
    // Enable to check that the tree is AVL balanced.
    inline constexpr bool CHECK_DEPTH = true;

    // Disable if your implementation does not have parent pointers
    inline constexpr bool PARENT_POINTERS = true;
}

// TODO implement
template < typename T >
struct Tree {
    struct Node;

    Tree() : root(nullptr), count(0) { }
    ~Tree() {
        recursionClear(root);
    }

    size_t size() const { return count ; }

    const T* find(const T& value) const {
        return recursionFind(value,root);
    }
    bool insert(T value) {
        size_t oldCount = count;
        root = recursionInsert(value,root, nullptr);
        return oldCount != count;
    }

    bool erase(const T& value) {
        size_t oldCount = count;
        root = recursionErase(value,root);
        return oldCount != count;
    }

    Node* recursionInsert(const T& value, Node* ptr, Node * ptrParent) {
        if(! ptr ) {
            ptr = new Node(value,ptrParent);
            ++count;
            return ptr;
        }
        else if(value == ptr->value) {
            return ptr;
        }
        else if(value < ptr->value) {
            ptr->left = recursionInsert(value, ptr->left, ptr);
        }
        else {
            ptr->right = recursionInsert(value, ptr->right, ptr);
        }


        ptr->height = getMaxHeight(ptr) + 1;
        return balanceTree(ptr);
    }

    const T* recursionFind(const T& value, Node* ptr) const{
        if(! ptr ) {
            return nullptr;
        }
        else if(value == ptr->value) {
            return &ptr->value;
        }
        else if(value < ptr->value) {
            return recursionFind(value, ptr->left);
        }
        return recursionFind(value, ptr->right);

    }

    Node* recursionErase(const T& value, Node* ptr) {
        if( !ptr ) {
            return nullptr;
        }
        else if(value == ptr->value){
            if( !ptr->left && !ptr->right) {
                delete ptr;
                --count;
                return nullptr;
            }
            else if( !ptr->left ) {
                Node * temp = ptr->right;
                temp->parent = ptr->parent;
                delete ptr;
                --count;
                return temp;
            }
            else if( !ptr->right ) {
                Node * temp = ptr->left;
                temp->parent = ptr->parent;
                delete ptr;
                --count;
                return temp;
            }
            else {
                Node * suc = findMin(ptr->right);
                ptr->value = suc->value;
                ptr->right = recursionErase(ptr->value,ptr->right);
            }
        }
        else if(value < ptr->value) {
            ptr->left = recursionErase(value, ptr->left);
        } else {
            ptr->right = recursionErase(value,ptr->right);
        }

        ptr->height = getMaxHeight(ptr) + 1;
        return balanceTree(ptr);
    }

    Node* findMin(Node * ptr) {
        if( ptr ) {
            if( ! ptr->left ){
                return ptr;
            } else {
                return findMin(ptr->left);
            }
        }
        return nullptr;
    }

    void recursionClear(Node * ptr) {
        if( ptr ) {
            recursionClear(ptr->left);
            recursionClear(ptr->right);
            delete ptr;
        }
    }

    Node* balanceTree(Node * ptr) {
        int vertexSymbol = getVertexSymbol(ptr);
        if(vertexSymbol < -1) {
            if(getVertexSymbol(ptr->left) > 0) {
                ptr->left = leftRotation(ptr->left);
                return rightRotation(ptr);
            }
            else {
                return rightRotation(ptr);

            }
        }
        else if(vertexSymbol > 1 ) {
            if(getVertexSymbol(ptr->right) < 0) {
                ptr->right = rightRotation(ptr->right);
                return leftRotation(ptr);
            }
            else {
                return leftRotation(ptr);
            }
        }

        return ptr;
    }

    int getVertexSymbol(Node * ptr) {
        int leftH = ptr->left ? ptr->left->height : 0;
        int rightH = ptr->right ? ptr->right->height : 0;
        return rightH - leftH;
    }

    int getMaxHeight(Node * ptr) {
        int leftH = ptr->left ? ptr->left->height : 0;
        int rightH = ptr->right ? ptr->right->height : 0;
        return leftH > rightH ? leftH : rightH;
    }

    Node* rightRotation(Node * ptr) {
        Node* y = ptr->left;
        ptr->left = y->right;
        y->right = ptr;
        if(ptr->left) {
            ptr->left->parent = ptr;
        }
        y->parent = ptr->parent;
        ptr->parent = y;

        ptr->height = getMaxHeight(ptr) + 1;
        y->height = getMaxHeight(y) + 1;

        return y;
    }

    Node * leftRotation(Node * ptr) {
        Node * y = ptr->right;
        ptr->right = y->left;
        y->left = ptr;
        if(ptr->right) {
            ptr->right->parent = ptr;
        }
        y->parent = ptr->parent;
        ptr->parent = y;

        ptr->height = getMaxHeight(ptr) + 1;
        y->height = getMaxHeight(y) + 1;
        return y;
    }

    struct Node {
        Node(const T& _value, Node * _parent)
            :   value(_value),
                parent(_parent),
                left(nullptr),
                right(nullptr),
                height(1)
        {}
        T value;
        Node* parent;
        Node* left;
        Node* right;
        int height;
    };
    Node * root;
    size_t count;

    struct TesterInterface {
        // using Node = ...
        static const Node *root(const Tree *t) { return t->root; }
        // Parent of root must be nullptr, ignore if config::PARENT_POINTERS == false
        static const Node *parent(const Node *n) { return n->parent; }
        static const Node *right(const Node *n) { return n->right; }
        static const Node *left(const Node *n) { return n->left; }
        static const T& value(const Node *n) { return n->value; }
    };
};


#ifndef __PROGTEST__

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

template < typename T >
struct Tester {
    Tester() = default;

    void size() const {
        size_t r = ref.size();
        size_t t = tested.size();
        if (r != t) throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
    }

    void find(const T& x) const {
        auto r = ref.find(x);
        auto t = tested.find(x);
        bool found_r = r != nullptr;
        bool found_t = t != nullptr;

        if (found_r != found_t) _throw("Find mismatch", found_r);
        if (found_r && *t != x) throw TestFailed("Find: found different value");
    }

    void insert(const T& x, bool check_tree_ = false) {
        auto succ_r = ref.insert(x);
        auto succ_t = tested.insert(x);
        if (succ_r != succ_t) _throw("Insert mismatch", succ_r);
        size();
        if (check_tree_) check_tree();
    }

    void erase(const T& x, bool check_tree_ = false) {
        bool succ_r = ref.erase(x);
        auto succ_t = tested.erase(x);
        if (succ_r != succ_t) _throw("Erase mismatch", succ_r);
        size();
        if (check_tree_) check_tree();
    }

    struct NodeCheckResult {
        const T* min = nullptr;
        const T* max = nullptr;
        int depth = -1;
        size_t size = 0;
    };

    void check_tree() const {
        using TI = typename Tree<T>::TesterInterface;
        auto ref_it = ref.begin();
        bool check_value_failed = false;
        auto check_value = [&](const T& v) {
            if (check_value_failed) return;
            check_value_failed = (ref_it == ref.end() || *ref_it != v);
            if (!check_value_failed) ++ref_it;
        };

        auto r = check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);
        size_t t_size = tested.size();

        if (t_size != r.size) throw TestFailed(
                    fmt("Check tree: size() reports %zu but expected %zu.", t_size, r.size));

        if (check_value_failed) throw TestFailed(
                    "Check tree: element mismatch");

        size();
    }

    template < typename Node, typename F >
    NodeCheckResult check_node(const Node* n, const Node* p, F& check_value) const {
        if (!n) return {};

        using TI = typename Tree<T>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p) throw TestFailed("Parent mismatch.");
        }

        auto l = check_node(TI::left(n), n, check_value);
        check_value(TI::value(n));
        auto r = check_node(TI::right(n), n, check_value);

        if (l.max && !(*l.max < TI::value(n)))
            throw TestFailed("Max of left subtree is too big.");
        if (r.min && !(TI::value(n) < *r.min))
            throw TestFailed("Min of right subtree is too small.");

        if (config::CHECK_DEPTH && abs(l.depth - r.depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l.depth, r.depth
            ));

        return {
                l.min ? l.min : &TI::value(n),
                r.max ? r.max : &TI::value(n),
                std::max(l.depth, r.depth) + 1, 1 + l.size + r.size
        };
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    Tree<T> tested;
    Ref<T> ref;
};


void test_insert() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 0; i < 10; i++) t.insert((1 + i * 7) % 17, true);
    for (int i = -10; i < 20; i++) t.find(i);
}

void test_erase() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert((1 + i * 7) % 17, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 3; i < 22; i += 2) t.erase(i, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = 0; i < 10; i++) t.insert((1 + i * 13) % 17 - 8, true);
    for (int i = -10; i < 20; i++) t.find(i);

    for (int i = -4; i < 10; i++) t.erase(i, true);
    for (int i = -10; i < 20; i++) t.find(i);
}

enum RandomTestFlags : unsigned {
    SEQ = 1, NO_ERASE = 2, CHECK_TREE = 4
};

void test_random(size_t size, unsigned flags = 0) {
    Tester<size_t> t;
    std::mt19937 my_rand(24707 + size);

    bool seq = flags & SEQ;
    bool erase = !(flags & NO_ERASE);
    bool check_tree = flags & CHECK_TREE;

    for (size_t i = 0; i < size; i++)
        t.insert(seq ? 2*i : my_rand() % (3*size), check_tree);

    t.check_tree();

    for (size_t i = 0; i < 3*size + 1; i++) t.find(i);

    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 5) {
            case 1: t.insert(my_rand() % (3*size), check_tree);
                break;
            case 2: if (erase) t.erase(my_rand() % (3*size), check_tree);
                break;
            default:
                t.find(my_rand() % (3*size));
        }

    t.check_tree();
}

int main() {
    try {
        std::cout << "Insert test..." << std::endl;
        test_insert();

        std::cout << "Erase test..." << std::endl;
        test_erase();

        std::cout << "Tiny random test..." << std::endl;
        test_random(20, CHECK_TREE);

        std::cout << "Small random test..." << std::endl;
        test_random(200, CHECK_TREE);

        std::cout << "Big random test..." << std::endl;
        test_random(50'000);

        std::cout << "Big sequential test..." << std::endl;
        test_random(50'000, SEQ);
        std::cout << "All tests passed." << std::endl;
    } catch (const TestFailed& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
    }
}

#endif


