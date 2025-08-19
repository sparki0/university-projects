#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;
#endif /* __PROGTEST__ */

template<typename T> class Vector;

template <typename T, typename Func>
int lowBound(const Vector<T>& arr, size_t size, const T& value,Func func)  {
    int left = 0;
    int right = (int)size - 1;
    while (left <= right) {
        int middle = left + (right - left) / 2;
        if (func(arr[middle], value) < 0) {
            left = middle + 1;
        } else {
            right = middle - 1;
        }
    }
    return left;
}

template<typename T>
class Vector{
public:
    Vector();
    ~Vector() { delete[] _ptr; }
    Vector& operator=(const Vector<T>& src);
    T& operator[](size_t index) { return _ptr[index]; }
    T& operator[](size_t index) const { return _ptr[index]; }
    size_t getSize() const {return  _size; };
    bool isEmpty() const { return !_size; };
    void insert(size_t index, const T& value);
    void pushBack(const T& value);
    bool isOnEnd(size_t size) const { return _size == size; }
    void clear();
private:
    size_t _size{};
    size_t _capacity{};
    T * _ptr;
    void addCapacity();
};
template<typename T> void Vector<T>::clear() {
    _size = 0;
    _capacity = 10;
    delete[] _ptr;
    _ptr = nullptr;
}


template<typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& src) {
    if (this != &src) {
        if (_capacity != src._capacity) {
            delete[] _ptr;
            _capacity = src._capacity;
            _ptr = new T[_capacity];
        }
        _size = src._size;
        for(size_t i = 0; i < _size; ++i){
            _ptr[i] = T(src[i]);
        }
    }
    return *this;
}

template<typename T> void Vector<T>::addCapacity() {
    _capacity *= 2;
    T* temp = new T[_capacity];
    for(size_t i = 0; i < _size; ++i){
        temp[i] = T(_ptr[i]);
    }
    delete[] _ptr;
    _ptr = temp;
}

template<typename T> Vector<T>::Vector(): _size(0), _capacity(10) {
    _ptr = new T[_capacity];
}
template<typename T> void Vector<T>::insert(size_t index, const T& value) {
    if (_size >= _capacity - 1) {
        addCapacity();
    }
    if(isOnEnd(index)){
        this->pushBack(value);
        return;
    }
    for(int i = (int)_size; i > (int)index; --i){
        _ptr[i] = T(_ptr[i-1]);
    }
    _ptr[index] = value;
    ++_size;
}

template<typename T> void Vector<T>::pushBack(const T& value) {
    if (_size >= _capacity - 1) {
        addCapacity();
    }
    _ptr[_size] = value;
    ++_size;
}

class String{
public:
    String() : _str(nullptr), _length(0){}
    String(const char * src);
    String(const String& src);
    ~String();
    String& operator=(const char* src);
    String& operator=(const String& src);
    bool operator==(const String& right) const;
    bool operator!=(const String& right) const;
    friend int cmpStrings(const String& a, const String& b);
    friend ostream & operator<<(ostream& out, const String& str){
        out << str._str;
        return out;
    }
private:
    char* _str;
    size_t _length;
};

String::String(const char * src){
    _length = strlen(src);
    _str = new char[_length + 1];
    strcpy(_str,src);
}
String::String(const String& src){
    _length = src._length;
    _str = new char[_length + 1];
    strcpy(_str,src._str);
}
String::~String(){
//    cout << _str << endl;
    delete[] _str;
}
String& String::operator=(const char* src){
    if(::strcmp(_str,src) != 0){
        delete[] _str;
        _length = strlen(src);
        _str = new char[_length + 1];
        strcpy(_str,src);
    }
    return *this;
}
String& String::operator=(const String& src){
    if(this != &src){
        delete[] _str;
        _length = strlen(src._str);
        _str = new char[_length + 1];
        strcpy(_str,src._str);
    }
    return *this;
}
int cmpStrings(const String& a, const String& b){
    return strcmp(a._str,b._str);
}
bool String::operator==(const String &right) const {
    return !(strcmp(_str,right._str));
}
bool String::operator!=(const String &right) const {
    return !(*this == right);
}

class CMail
{
    friend class CMailServer;
public:
    CMail() {};
    CMail(const char * from, const char * to, const char * body );
    CMail& operator=(const CMail& src);
    CMail(const CMail& src);
    bool operator==(const CMail& x ) const;
    friend ostream& operator<<( ostream& os, const CMail & m );
private:
    String _from;
    String _to;
    String _body;
};


CMail::CMail(const char *from, const char *to, const char *body){
    _from = String(from);
    _to = String(to);
    _body = String(body);
}
CMail &CMail::operator=(const CMail &src) {
    if (this != &src){
        _from = src._from;
        _to = src._to;
        _body = src._body;
    }
    return *this;
}

CMail::CMail(const CMail& src){
    _from = src._from;
    _to = src._to;
    _body = src._body;
}

bool CMail::operator==(const CMail &x) const {
    return (_from == x._from && _to == x._to && _body == x._body);
}

ostream &operator<<(ostream &os, const CMail &m) {
    os << "From: " << m._from << ", To: " << m._to << ", Body: " << m._body;
    return os;
}



class CMailIterator
{
    friend int main();
    friend class CMailServer;

public:
    CMailIterator(): _ind(-1) {};
    explicit CMailIterator(const Vector<CMail>& mails) {
        for(size_t i = 0; i < mails.getSize(); ++i){
            _mails.pushBack(mails[i]);
        }
        _ind = mails.isEmpty() ? -1 : 0;
    };
    void addMails(const Vector<CMail>& mails){
        for(size_t i = 0; i < mails.getSize(); ++i){
            _mails.pushBack(mails[i]);
        }
        _ind = mails.isEmpty() ? -1 : 0;
    }
    explicit operator bool(void) const;
    bool operator !(void) const;
    const CMail & operator *(void) const;
    CMailIterator & operator++( void );
private:
    int _ind;
    Vector<CMail> _mails;
};

CMailIterator::operator bool(void) const {
    if(_ind != -1 && !_mails.isOnEnd(_ind)){
        return true;
    } else {
        return false;
    }
}

bool CMailIterator::operator!(void) const {
    return !(this->operator bool());
}

const CMail &CMailIterator::operator*(void) const {
    return _mails[_ind];
}

CMailIterator &CMailIterator::operator++(void) {
    if(_ind != -1 && !_mails.isOnEnd(_ind)){
        ++_ind;
    }
    return *this;
}

class Pair{
public:
    friend class CMailServer;
    friend int main();
    Pair() {}
    Pair(const String& str): _name(str) {}
    Pair(const Pair& src)  {
        _name = src._name;
        _mails = src._mails;
    }

    friend int cmpPair(const Pair& a, const Pair& b);
    friend ostream& operator<<(ostream& out, const Pair& a){
        out << a._name;
        return out;
    }
private:
    String _name;
    Vector<CMail> _mails;
};


int cmpPair(const Pair& a, const Pair& b){
    return cmpStrings(a._name,b._name);
}
class CMailServer
{
    friend int main();
public:
    CMailServer(void);
    CMailServer(const CMailServer & src);
    CMailServer& operator=(const CMailServer & src);
    ~CMailServer( void );
    void sendMail(const CMail & m );
    CMailIterator outbox(const char * email) const;
    CMailIterator inbox(const char * email) const;
private:
    Vector<Pair> _outbox;
    Vector<Pair> _inbox;
    void addOutbox(const CMail &m);
    void addInbox(const CMail &m);
};

CMailServer::CMailServer(void) {
}
CMailServer::~CMailServer(void) {
}

CMailServer::CMailServer(const CMailServer &src) {
    _outbox = src._outbox;
    _inbox = src._inbox;
}

CMailServer &CMailServer::operator=(const CMailServer &src) {
    if(this != &src){
//        _outbox.clear();
//        _inbox.clear();
        _outbox = src._outbox;
        _inbox = src._inbox;
    }

    return *this;
}

void CMailServer::sendMail(const CMail &m) {
    addOutbox(m);
    addInbox(m);
}

void CMailServer::addOutbox(const CMail &m) {
    Pair pair(m._from);
    auto ind = lowBound(_outbox,_outbox.getSize(),pair,cmpPair);
    if(!_outbox.isOnEnd(ind) && _outbox[ind]._name == m._from){
        _outbox[ind]._mails.pushBack(m);
    } else {
        _outbox.insert(ind,pair._name);
        _outbox[ind]._mails.pushBack(m);
    }
}

void CMailServer::addInbox(const CMail &m) {
    Pair pair(m._to);
    auto ind = lowBound(_inbox,_inbox.getSize(),pair,cmpPair);
    if(!_inbox.isOnEnd(ind) && _inbox[ind]._name == m._to){
        _inbox[ind]._mails.pushBack(m);
    } else {
        _inbox.insert(ind,pair._name);
        _inbox[ind]._mails.pushBack(m);
    }
}

CMailIterator CMailServer::outbox(const char *email) const {
    String str(email);
    Pair pair(str);
    CMailIterator res;
    auto ind = lowBound(_outbox,_outbox.getSize(),pair,cmpPair);
    if(!_outbox.isOnEnd(ind) && _outbox[ind]._name == str) {
        res.addMails(_outbox[ind]._mails);
    }
    return res;
}

CMailIterator CMailServer::inbox(const char *email) const {
    String str(email);
    Pair pair(str);
    CMailIterator res;
    auto ind = lowBound(_inbox,_inbox.getSize(),pair,cmpPair);
    if(!_inbox.isOnEnd(ind) && _inbox[ind]._name == str){
        res.addMails(_inbox[ind]._mails);
    }
    return res;
}



#ifndef __PROGTEST__
bool                         matchOutput                   ( const CMail     & m,
                                                             const char      * str )
{
    ostringstream oss;
    oss << m;
    return oss . str () == str;
}

int main ( void )
{
    char from[100], to[100], body[1024];

    assert ( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "john", "peter", "progtest deadline" ) );
    assert ( !( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "john", "progtest deadline", "peter" ) ) );
    assert ( !( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "peter", "john", "progtest deadline" ) ) );
    assert ( !( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "peter", "progtest deadline", "john" ) ) );
    assert ( !( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "progtest deadline", "john", "peter" ) ) );
    assert ( !( CMail ( "john", "peter", "progtest deadline" ) == CMail ( "progtest deadline", "peter", "john" ) ) );
    CMailServer s0;
    s0 . sendMail ( CMail ( "john", "peter", "some important mail" ) );
    strncpy ( from, "john", sizeof ( from ) );
    strncpy ( to, "thomas", sizeof ( to ) );
    strncpy ( body, "another important mail", sizeof ( body ) );
    s0 . sendMail ( CMail ( from, to, body ) );

    strncpy ( from, "john", sizeof ( from ) );
    strncpy ( to, "alice", sizeof ( to ) );
    strncpy ( body, "deadline notice", sizeof ( body ) );
    s0 . sendMail ( CMail ( from, to, body ) );
    s0 . sendMail ( CMail ( "alice", "john", "deadline confirmation" ) );
    s0 . sendMail ( CMail ( "peter", "alice", "PR bullshit" ) );
    CMailIterator i0 = s0 . inbox ( "alice" );
    assert ( i0 && *i0 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i0,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i0 && *i0 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i0,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ! ++i0 );

    CMailIterator i1 = s0 . inbox ( "john" );
    assert ( i1 && *i1 == CMail ( "alice", "john", "deadline confirmation" ) );
    assert ( matchOutput ( *i1,  "From: alice, To: john, Body: deadline confirmation" ) );
    assert ( ! ++i1 );

    CMailIterator i2 = s0 . outbox ( "john" );
    assert ( i2 && *i2 == CMail ( "john", "peter", "some important mail" ) );
    assert ( matchOutput ( *i2,  "From: john, To: peter, Body: some important mail" ) );
    assert ( ++i2 && *i2 == CMail ( "john", "thomas", "another important mail" ) );
    assert ( matchOutput ( *i2,  "From: john, To: thomas, Body: another important mail" ) );
    assert ( ++i2 && *i2 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i2,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ! ++i2 );

    CMailIterator i3 = s0 . outbox ( "thomas" );
    assert ( ! i3 );

    CMailIterator i4 = s0 . outbox ( "steve" );
    assert ( ! i4 );

    CMailIterator i5 = s0 . outbox ( "thomas" );
    s0 . sendMail ( CMail ( "thomas", "boss", "daily report" ) );
    assert ( ! i5 );


    CMailIterator i6 = s0 . outbox ( "thomas" );
    assert ( i6 && *i6 == CMail ( "thomas", "boss", "daily report" ) );
    assert ( matchOutput ( *i6,  "From: thomas, To: boss, Body: daily report" ) );
    assert ( ! ++i6 );

    CMailIterator i7 = s0 . inbox ( "alice" );
    s0 . sendMail ( CMail ( "thomas", "alice", "meeting details" ) );
    assert ( i7 && *i7 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i7,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i7 && *i7 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i7,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ! ++i7 );

    CMailIterator i8 = s0 . inbox ( "alice" );
    assert ( i8 && *i8 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i8,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i8 && *i8 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i8,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ++i8 && *i8 == CMail ( "thomas", "alice", "meeting details" ) );
    assert ( matchOutput ( *i8,  "From: thomas, To: alice, Body: meeting details" ) );
    assert ( ! ++i8 );

    CMailServer s1 ( s0 );
    s0 . sendMail ( CMail ( "joe", "alice", "delivery details" ) );
    s1 . sendMail ( CMail ( "sam", "alice", "order confirmation" ) );
    CMailIterator i9 = s0 . inbox ( "alice" );
    assert ( i9 && *i9 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i9,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i9 && *i9 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i9,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ++i9 && *i9 == CMail ( "thomas", "alice", "meeting details" ) );
    assert ( matchOutput ( *i9,  "From: thomas, To: alice, Body: meeting details" ) );
    assert ( ++i9 && *i9 == CMail ( "joe", "alice", "delivery details" ) );
    assert ( matchOutput ( *i9,  "From: joe, To: alice, Body: delivery details" ) );
    assert ( ! ++i9 );

    CMailIterator i10 = s1 . inbox ( "alice" );
    assert ( i10 && *i10 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i10,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i10 && *i10 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i10,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ++i10 && *i10 == CMail ( "thomas", "alice", "meeting details" ) );
    assert ( matchOutput ( *i10,  "From: thomas, To: alice, Body: meeting details" ) );
    assert ( ++i10 && *i10 == CMail ( "sam", "alice", "order confirmation" ) );
    assert ( matchOutput ( *i10,  "From: sam, To: alice, Body: order confirmation" ) );
    assert ( ! ++i10 );

    CMailServer s2;
    s2 . sendMail ( CMail ( "alice", "alice", "mailbox test" ) );
    CMailIterator i11 = s2 . inbox ( "alice" );
    assert ( i11 && *i11 == CMail ( "alice", "alice", "mailbox test" ) );
    assert ( matchOutput ( *i11,  "From: alice, To: alice, Body: mailbox test" ) );
    assert ( ! ++i11 );

    s2 = s0;
    s0 . sendMail ( CMail ( "steve", "alice", "newsletter" ) );
    s2 . sendMail ( CMail ( "paul", "alice", "invalid invoice" ) );
    CMailIterator i12 = s0 . inbox ( "alice" );
    assert ( i12 && *i12 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i12,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i12 && *i12 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i12,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ++i12 && *i12 == CMail ( "thomas", "alice", "meeting details" ) );
    assert ( matchOutput ( *i12,  "From: thomas, To: alice, Body: meeting details" ) );
    assert ( ++i12 && *i12 == CMail ( "joe", "alice", "delivery details" ) );
    assert ( matchOutput ( *i12,  "From: joe, To: alice, Body: delivery details" ) );
    assert ( ++i12 && *i12 == CMail ( "steve", "alice", "newsletter" ) );
    assert ( matchOutput ( *i12,  "From: steve, To: alice, Body: newsletter" ) );
    assert ( ! ++i12 );

    CMailIterator i13 = s2 . inbox ( "alice" );
    assert ( i13 && *i13 == CMail ( "john", "alice", "deadline notice" ) );
    assert ( matchOutput ( *i13,  "From: john, To: alice, Body: deadline notice" ) );
    assert ( ++i13 && *i13 == CMail ( "peter", "alice", "PR bullshit" ) );
    assert ( matchOutput ( *i13,  "From: peter, To: alice, Body: PR bullshit" ) );
    assert ( ++i13 && *i13 == CMail ( "thomas", "alice", "meeting details" ) );
    assert ( matchOutput ( *i13,  "From: thomas, To: alice, Body: meeting details" ) );
    assert ( ++i13 && *i13 == CMail ( "joe", "alice", "delivery details" ) );
    assert ( matchOutput ( *i13,  "From: joe, To: alice, Body: delivery details" ) );
    assert ( ++i13 && *i13 == CMail ( "paul", "alice", "invalid invoice" ) );
    assert ( matchOutput ( *i13,  "From: paul, To: alice, Body: invalid invoice" ) );
    assert ( ! ++i13 );

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
