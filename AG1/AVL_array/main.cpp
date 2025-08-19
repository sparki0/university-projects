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

// We use std::vector as a reference to check our implementation.
// It is not available in progtest :)
#include <vector>

template < typename T >
struct Ref {
    bool empty() const { return _data.empty(); }
    size_t size() const { return _data.size(); }

    const T& operator [] (size_t index) const { return _data.at(index); }
    T& operator [] (size_t index) { return _data.at(index); }

    void insert(size_t index, T value) {
        if (index > _data.size()) throw std::out_of_range("oops");
        _data.insert(_data.begin() + index, std::move(value));
    }

    T erase(size_t index) {
        T ret = std::move(_data.at(index));
        _data.erase(_data.begin() + index);
        return ret;
    }

    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }

private:
    std::vector<T> _data;
};

#endif


namespace config {
    inline constexpr bool PARENT_POINTERS = true;
    inline constexpr bool CHECK_DEPTH = true;
}

// TODO implement
template < typename T >
struct Array {
    struct Node;
    Array() : root(nullptr), count(0) {}
    ~Array() {
        recursionClear(root);
    }
    void recursionClear(Node * ptr) {
        if( ptr ) {
            recursionClear(ptr->left);
            recursionClear(ptr->right);
            delete ptr;
        }
    }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }

    const T& operator [] (size_t index) const {
        return findByIndex(root,index);
    }
    T& operator [] (size_t index) {
        if(index >= count) {
            throw std::out_of_range("");
        }
        return findByIndex(root,index);
    }

    void insert(size_t index, T value) {
        if(index > count) {
            throw std::out_of_range("");
        }
        root = recursionInsert(value,index,root, nullptr);

    }

    T erase(size_t index) {
        if(index > count) {
            throw std::out_of_range("");
        }
        T result;
        root = recursionErase(index,root,result);
        return result;
    }


    T& findByIndex(Node * ptr, size_t index) const{
        size_t countLeft = getCountOfSubTree(ptr->left);

        if(index < countLeft) {
            return findByIndex(ptr->left, index);
        }
        else if(index > countLeft) {
            return findByIndex(ptr->right, index - countLeft - 1);
        }
        return ptr->value;
    }

    Node* recursionInsert(const T& value, size_t index , Node* ptr, Node * ptrParent) {
        if(! ptr ) {
            ptr = new Node(value,ptrParent);
            ++count;
            return ptr;
        }
        size_t countLeft = getCountOfSubTree(ptr->left);
        if(index <= countLeft) {
            ptr->left = recursionInsert(value,index,ptr->left,ptr);
        }
        else if(index > countLeft) {
            ptr->right = recursionInsert(value,index - countLeft - 1,ptr->right,ptr);
        }

        ptr->height = getMaxHeight(ptr) + 1;
        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;

        return balanceTree(ptr);
    }

    Node* recursionErase(size_t index,Node* ptr, T & result) {
        if( !ptr ) {
            return nullptr;
        }
        size_t leftCount = getCountOfSubTree(ptr->left);
        if(index == leftCount) {
            if( !ptr->left && !ptr->right ) {
                result = ptr->value;
                delete ptr;
                --count;
                return nullptr;
            }
            else if( !ptr->left ) {
                Node * temp = ptr->right;
                temp->parent = ptr->parent;
                result = ptr->value;
                delete ptr;
                --count;
                return temp;
            }
            else if( !ptr->right ) {
                Node * temp = ptr->left;
                temp->parent = ptr->parent;
                result = ptr->value;
                delete ptr;
                --count;
                return temp;
            }
            else {
                Node * suc = findMin(ptr->right);
                ptr->value = suc->value;
                ptr->right = recursionErase(0 ,ptr->right,result);
            }
        }
        else if(index < leftCount) {
            ptr->left = recursionErase(index, ptr->left,result);
        } else {
            ptr->right = recursionErase(index - leftCount - 1,ptr->right,result);
        }

        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
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

    size_t getCountOfSubTree(Node * ptr) const{
        return ptr ? ptr->nodeCount : 0;
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

    int getSumOfNodeCount(Node * ptr) {
        int leftH = ptr->left ? ptr->left->nodeCount : 0;
        int rightH = ptr->right ? ptr->right->nodeCount : 0;
        return rightH + leftH;
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
        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
        y->nodeCount = getSumOfNodeCount(y) + 1;

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
        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
        y->nodeCount = getSumOfNodeCount(y) + 1;
        return y;
    }

    struct Node {
        Node(const T& _value, Node * _parent)
                :   value(_value),
                    parent(_parent),
                    left(nullptr),
                    right(nullptr),
                    height(1),
                    nodeCount(1)
        {}
        T value;
        Node* parent;
        Node* left;
        Node* right;
        int height;
        size_t nodeCount;
    };
    Node * root;
    size_t count;
    struct TesterInterface {
        // using Node = ...
        static const Node *root(const Array *t) { return t->root; }
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

    size_t size() const {
        bool te = tested.empty();
        size_t r = ref.size();
        size_t t = tested.size();
        if (te != !t) throw TestFailed(fmt("Size: size %zu but empty is %s.",
                                           t, te ? "true" : "false"));
        if (r != t) throw TestFailed(fmt("Size: got %zu but expected %zu.", t, r));
        return r;
    }

    const T& operator [] (size_t index) const {
        const T& r = ref[index];
        const T& t = tested[index];
        if (r != t) throw TestFailed("Op [] const mismatch.");
        return t;
    }

    void assign(size_t index, T x) {
        ref[index] = x;
        tested[index] = std::move(x);
        operator[](index);
    }

    void insert(size_t i, T x, bool check_tree_ = false) {
        ref.insert(i, x);
        tested.insert(i, std::move(x));
        size();
        if (check_tree_) check_tree();
    }

    T erase(size_t i, bool check_tree_ = false) {
        T r = ref.erase(i);
        T t = tested.erase(i);
        if (r != t) TestFailed(fmt("Erase mismatch at %zu.", i));
        size();
        if (check_tree_) check_tree();
        return t;
    }

    void check_tree() const {
        using TI = typename Array<T>::TesterInterface;
        auto ref_it = ref.begin();
        bool check_value_failed = false;
        auto check_value = [&](const T& v) {
            if (check_value_failed) return;
            check_value_failed = (ref_it == ref.end() || *ref_it != v);
            if (!check_value_failed) ++ref_it;
        };

        size();

        check_node(TI::root(&tested), decltype(TI::root(&tested))(nullptr), check_value);

        if (check_value_failed) throw TestFailed(
                    "Check tree: element mismatch");
    }

    template < typename Node, typename F >
    int check_node(const Node* n, const Node* p, F& check_value) const {
        if (!n) return -1;

        using TI = typename Array<T>::TesterInterface;
        if constexpr(config::PARENT_POINTERS) {
            if (TI::parent(n) != p) throw TestFailed("Parent mismatch.");
        }

        auto l_depth = check_node(TI::left(n), n, check_value);
        check_value(TI::value(n));
        auto r_depth = check_node(TI::right(n), n, check_value);

        if (config::CHECK_DEPTH && abs(l_depth - r_depth) > 1) throw TestFailed(fmt(
                    "Tree is not avl balanced: left depth %i and right depth %i.",
                    l_depth, r_depth
            ));

        return std::max(l_depth, r_depth) + 1;
    }

    static void _throw(const char *msg, bool s) {
        throw TestFailed(fmt("%s: ref %s.", msg, s ? "succeeded" : "failed"));
    }

    Array<T> tested;
    Ref<T> ref;
};


void test_insert() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(15, (1 + i * 7) % 17, true);
    for (int i = 0; i < 10; i++) t.assign(2*i, 3*t[2*i]);
    for (size_t i = 0; i < t.size(); i++) t[i];
}

void test_erase() {
    Tester<int> t;

    for (int i = 0; i < 10; i++) t.insert(i, i, true);
    for (int i = 0; i < 10; i++) t.insert(i, -i, true);

    for (size_t i = 3; i < t.size(); i += 2) t.erase(i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];

    for (int i = 0; i < 5; i++) t.insert(3, (1 + i * 7) % 17, true);
    for (size_t i = 1; i < t.size(); i += 3) t.erase(i, true);

    for (int i = 0; i < 20; i++) t.insert(3, 100 + i, true);

    for (int i = 0; i < 5; i++) t.erase(t.size() - 1, true);
    for (int i = 0; i < 5; i++) t.erase(0, true);

    for (int i = 0; i < 4; i++) t.insert(i, i, true);
    for (size_t i = 0; i < t.size(); i++) t[i];
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

    for (size_t i = 0; i < size; i++) {
        size_t pos = seq ? 0 : my_rand() % (i + 1);
        t.insert(pos, my_rand() % (3*size), check_tree);
    }

    t.check_tree();

    for (size_t i = 0; i < t.size(); i++) t[i];

    for (size_t i = 0; i < 30*size; i++) switch (my_rand() % 7) {
            case 1: {
                if (!erase && i % 3 == 0) break;
                size_t pos = seq ? 0 : my_rand() % (t.size() + 1);
                t.insert(pos, my_rand() % 1'000'000, check_tree);
                break;
            }
            case 2:
                if (erase) t.erase(my_rand() % t.size(), check_tree);
                break;
            case 3:
                t.assign(my_rand() % t.size(), 155 + i);
                break;
            default:
                t[my_rand() % t.size()];
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

        std::cout << "Bigger random test..." << std::endl;
        test_random(5'000);

        std::cout << "Bigger sequential test..." << std::endl;
        test_random(5'000, SEQ);
        std::cout << "All tests passed." << std::endl;
    } catch (const TestFailed& e) {
        std::cout << "Test failed: " << e.what() << std::endl;
    }
}

#endif


