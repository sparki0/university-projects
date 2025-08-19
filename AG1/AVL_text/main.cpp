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
#include <stack>
#include <queue>
#include <random>
#include <type_traits>

#endif

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

    const char& operator [] (size_t index) const {
        if(index >= count) {
            throw std::out_of_range("");
        }
        return findByIndex(root,index);
    }
    char& operator [] (size_t index) {
        if(index >= count) {
            throw std::out_of_range("");
        }
        return findByIndex(root,index);
    }

    size_t lineIndexChar(size_t ind) const {
        return recursionLineIndexChar(ind,0,root);
    }

    size_t recursionLineIndexChar(size_t ind, size_t countNewLine, Node * ptr) const {
        size_t leftCount = getCountOfSubTree(ptr->left);
        if(ind < leftCount) {
            return recursionLineIndexChar(ind,countNewLine, ptr->left);
        }
        else if (ind > leftCount) {
            countNewLine += getSumOfNewLineCount(ptr->left);
            if(ptr->value == '\n') {
                countNewLine++;
            }
            return recursionLineIndexChar(ind - getCountOfSubTree(ptr->left) - 1,countNewLine, ptr->right);
        }

        return countNewLine + getSumOfNewLineCount(ptr->left);

    }

    size_t lineStart(size_t r) const {
        if(r == 0) {
            return 0;
        }
        return findIndexNewLine(root,r - 1,0) + 1;
    }

    size_t findIndexNewLine(Node * ptr, size_t r, size_t countInd) const {
        size_t newLineCountLeft = getSumOfNewLineCount(ptr->left);
        if(ptr->value == '\n' && newLineCountLeft == r) {
            return getCountOfSubTree(ptr->left) + countInd;
        }
        else if(r < newLineCountLeft) {
            return findIndexNewLine(ptr->left,r,countInd);
        }
        else {
            if(ptr->value == '\n'){
                ++newLineCountLeft;
            }
            return findIndexNewLine(ptr->right, r - newLineCountLeft, countInd + getCountOfSubTree(ptr->left) + 1);
        }
    }

    void edit(size_t index, char value) {
        if(index >= count) {
            throw std::out_of_range("");
        }
        recursionEdit(root,index,value);
    }

    void insert(size_t index, char value) {
        if(index > count) {
            throw std::out_of_range("");
        }
        root = recursionInsert(value,index,root, nullptr);
    }

    char erase(size_t index) {
        if(index >= count) {
            throw std::out_of_range("");
        }
        char result;
        root = recursionErase(index,root, result);
        return result;
    }

    char& findByIndex(Node * ptr, size_t index) const {
        size_t countLeft = getCountOfSubTree(ptr->left);

        if(index < countLeft) {
            return findByIndex(ptr->left, index);
        }
        else if(index > countLeft) {
            return findByIndex(ptr->right, index - countLeft - 1);
        }
        return ptr->value;
    }

    int getSumOfNewLineCount(Node * ptr) const {
        return ptr ? ptr->newLineCount : 0;
    }

    Node* recursionInsert(const char& value, size_t index , Node* ptr, Node * ptrParent) {
        if(! ptr ) {
            ptr = new Node(value,ptrParent);
            if(value == '\n') {
                ptr->newLineCount++;
            }
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
        ptr->newLineCount = getSumOfNewLineCount(ptr->left) + getSumOfNewLineCount(ptr->right);
        if(ptr->value == '\n') {
            ptr->newLineCount++;
        }
        return balanceTree(ptr);
    }

    Node* recursionErase(size_t index,Node* ptr, char & result) {
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
                ptr->right = recursionErase(0 ,ptr->right, result);
            }
        }
        else if(index < leftCount) {
            ptr->left = recursionErase(index, ptr->left, result);
        } else {
            ptr->right = recursionErase(index - leftCount - 1,ptr->right, result);
        }

        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
        ptr->height = getMaxHeight(ptr) + 1;
        ptr->newLineCount = getSumOfNewLineCount(ptr->left) + getSumOfNewLineCount(ptr->right);
        if(ptr->value == '\n') {
            ptr->newLineCount++;
        }
        return balanceTree(ptr);
    }

    void recursionEdit(Node * ptr, size_t index, char c) {
        size_t countLeft = getCountOfSubTree(ptr->left);

        if(index < countLeft) {
            recursionEdit(ptr->left, index, c);
        }
        else if(index > countLeft) {
            recursionEdit(ptr->right, index - countLeft - 1, c);
        }
        else {
            ptr->value = c;
        }
        ptr->newLineCount = getSumOfNewLineCount(ptr->left) + getSumOfNewLineCount(ptr->right);
        if(ptr->value == '\n') {
            ptr->newLineCount++;
        }
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

    int getSumOfNodeCount(Node * ptr) const {
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
        ptr->newLineCount = getSumOfNewLineCount(ptr->left) + getSumOfNewLineCount(ptr->right);
        if(ptr->value == '\n') {
            ptr->newLineCount++;
        }
        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
        y->nodeCount = getSumOfNodeCount(y) + 1;
        y->newLineCount = getSumOfNewLineCount(y->left) + getSumOfNewLineCount(y->right);
        if(y->value == '\n') {
            y->newLineCount++;
        }

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
        ptr->newLineCount = getSumOfNewLineCount(ptr->left) + getSumOfNewLineCount(ptr->right);
        if(ptr->value == '\n') {
            ptr->newLineCount++;
        }
        ptr->nodeCount = getSumOfNodeCount(ptr) + 1;
        y->nodeCount = getSumOfNodeCount(y) + 1;
        y->newLineCount = getSumOfNewLineCount(y->left) + getSumOfNewLineCount(y->right);
        if(y->value == '\n') {
            y->newLineCount++;
        }
        return y;
    }

    struct Node {
        Node(const char& _value, Node * _parent)
                :   value(_value),
                    parent(_parent),
                    left(nullptr),
                    right(nullptr),
                    height(1),
                    nodeCount(1),
                    newLineCount(0)
        {}
        char value;
        Node* parent;
        Node* left;
        Node* right;
        int height;
        size_t nodeCount;
        size_t newLineCount;
    };
    Node * root;
    size_t count;
    size_t _linesCount;
};


struct TextEditorBackend {
    TextEditorBackend(const std::string& text) {
        for(size_t i = 0; i < text.size(); ++i)  {
            _arr.insert(i,text[i]);
        }
    }

    size_t size() const { return _arr.count;}
    size_t lines() const {return _arr.root->newLineCount + 1; }

    char at(size_t i) const {
        return _arr[i];
    }
    void edit(size_t i, char c) {
        _arr.edit(i,c);
    }

    void insert(size_t i, char c) {
        _arr.insert(i,c);
    }

    void erase(size_t i) {
        _arr.erase(i);
    }

    size_t line_start(size_t r) const {
        if(r >= _arr.root->newLineCount + 1) {
            throw std::out_of_range("line_start");
        }

        return _arr.lineStart(r);

    }
    size_t line_length(size_t r) const {
        if(r >= _arr.root->newLineCount + 1) {
            throw std::out_of_range("line_length");
        }
        size_t start = line_start(r);
        size_t end;
        if(r + 1 >= _arr.root->newLineCount + 1) {
            end = _arr.count;
        }
        else {
            end = line_start(r + 1);
        }

        return end - start;

    }
    size_t char_to_line(size_t i) const{
        if(i >= _arr.count) {
            throw std::out_of_range("char_to_line");
        }
        return _arr.lineIndexChar(i);
    }

    Array _arr;
    size_t _linesCount;
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
    std::string ret = "\"";
    for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
    return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

#define CHECK_ALL(expr, ...) do { \
    std::array _arr = { __VA_ARGS__ }; \
    for (size_t _i = 0; _i < _arr.size(); _i++) \
      CHECK_((expr)(_i), _arr[_i], STR(expr) "(" << _i << ")", _arr[_i]); \
  } while (0)

#define CHECK_EX(expr, ex) do { \
    try { \
      (expr); \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but no exception was raised." << std::endl; \
    } catch (const ex&) { ok++; \
    } catch (...) { \
      fail++; \
      std::cout << "Line " << __LINE__ << ": Expected " STR(expr) \
        " to throw " #ex " but got different exception." << std::endl; \
    } \
  } while (0)

////////////////// End of dark magic ////////////////////////


std::string text(const TextEditorBackend& t) {
    std::string ret;
    for (size_t i = 0; i < t.size(); i++) ret.push_back(t.at(i));
    return ret;
}

void test1(int& ok, int& fail) {
    TextEditorBackend s("123\n456\n789");
    CHECK(s.size(), 11);
    CHECK(text(s), "123\n456\n789");
    CHECK(s.lines(), 3);
    CHECK_ALL(s.char_to_line, 0,0,0,0, 1,1,1,1, 2,2,2);
    CHECK_ALL(s.line_start, 0, 4, 8);
    CHECK_ALL(s.line_length, 4, 4, 3);
}

void test2(int& ok, int& fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK(t.size(), 12);
    CHECK(text(t), "123\n456\n789\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.char_to_line, 0,0,0,0, 1,1,1,1, 2,2,2,2);
    CHECK_ALL(t.line_start, 0, 4, 8, 12);
    CHECK_ALL(t.line_length, 4, 4, 4, 0);
}

void test3(int& ok, int& fail) {
    TextEditorBackend t("asdfasdfasdf");

    CHECK(t.size(), 12);
    CHECK(text(t), "asdfasdfasdf");
    CHECK(t.lines(), 1);
    CHECK_ALL(t.char_to_line, 0,0,0,0, 0,0,0,0, 0,0,0,0);
    CHECK(t.line_start(0), 0);
    CHECK(t.line_length(0), 12);

    t.insert(0, '\n');
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasdfasdf");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);

    t.insert(4, '\n');
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasdfasdf");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.insert(t.size(), '\n');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdf\n");
    CHECK(t.lines(), 4);
    CHECK_ALL(t.line_start, 0, 1, 5, 15);

    t.edit(t.size() - 1, 'H');
    CHECK(t.size(), 15);
    CHECK(text(t), "\nasd\nfasdfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(8);
    CHECK(t.size(), 14);
    CHECK(text(t), "\nasd\nfasfasdfH");
    CHECK(t.lines(), 3);
    CHECK_ALL(t.line_start, 0, 1, 5);

    t.erase(4);
    CHECK(t.size(), 13);
    CHECK(text(t), "\nasdfasfasdfH");
    CHECK(t.lines(), 2);
    CHECK_ALL(t.line_start, 0, 1);
}

void test_ex(int& ok, int& fail) {
    TextEditorBackend t("123\n456\n789\n");
    CHECK_EX(t.at(12), std::out_of_range);

    CHECK_EX(t.insert(13, 'a'), std::out_of_range);
    CHECK_EX(t.edit(12, 'x'), std::out_of_range);
    CHECK_EX(t.erase(12), std::out_of_range);

    CHECK_EX(t.line_start(4), std::out_of_range);
    CHECK_EX(t.line_start(40), std::out_of_range);
    CHECK_EX(t.line_length(4), std::out_of_range);
    CHECK_EX(t.line_length(6), std::out_of_range);
    CHECK_EX(t.char_to_line(12), std::out_of_range);
    CHECK_EX(t.char_to_line(25), std::out_of_range);
}

int main() {
    int ok = 0, fail = 0;
    if (!fail) test1(ok, fail);
    if (!fail) test2(ok, fail);
    if (!fail) test3(ok, fail);
    if (!fail) test_ex(ok, fail);

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


