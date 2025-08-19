#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <stack>
#include <queue>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <iterator>
#include <stdexcept>
using namespace std;
#endif /* __PROGTEST__ */

template <typename M_>
class CContest;

class Graph{
public:
    Graph& add(const string& str1, const string& str2){
        _vertexes[str1].insert(str2);
        return *this;
    }
    map<string,set<string>> _vertexes;

private:
};

template <typename T>
class Item
{
    friend class CContest<T>;
public:
    Item(const string& contestant1, const string& contestant2,const T& result)
        : _contestant1(contestant1), _contestant2(contestant2), _result(result)
    {
    };

    bool operator<(const Item& right) const {
        if(_contestant1 != right._contestant1){
            return _contestant1 < right._contestant1;
        }
        return _contestant2 < right._contestant2;
    }
private:
    string _contestant1;
    string _contestant2;
    T _result;
};

template <typename M_>
class CContest
{
public:
    CContest() = default;
    ~CContest() = default;

    CContest<M_>& addMatch(const string& contestant1, const string& contestant2,const M_& result);

    template <typename Func>
    bool isOrdered(Func func) const;

    template <typename Func>
    list<string> results(Func func) const;

private:
    template <typename Func>
    list<string> getList(Func func) const;

    set<Item<M_>> _data;

};

template<typename M_>
CContest<M_> &CContest<M_>::addMatch(const string &contestant1, const string &contestant2, const M_ &result) {
    Item newItemN(contestant1,contestant2, result);
    Item newItemR(contestant2,contestant1, result);
    if( contestant1 == contestant2 || _data.find(newItemN) != _data.end() || _data.find(newItemR) != _data.end()){
        throw logic_error("ne");
    }
    _data.insert(newItemN);
    return *this;
}

template<typename M_>

template <typename Func>
list<string> CContest<M_>::getList(Func func) const{
    list<string> res;
    string lox;
    map<string,int> loxM;
    for(auto& x: _data){
        loxM[x._contestant1] = 0;
        loxM[x._contestant2] = 0;
    }


    Graph graph;
    for(const auto& x: _data){
        if(func(x._result) > 0){
            graph.add(x._contestant2,x._contestant1);
            loxM[x._contestant1]++;
        } else if(func(x._result) < 0){
            graph.add(x._contestant1,x._contestant2);
            loxM[x._contestant2]++;
        } else {
            return res;
        }
    }

    int proverka = 0;
    for(auto &x: loxM){
        if(x.second == 0){
            lox = x.first;
            proverka++;
            if(proverka > 1){
                return res;
            }
        }
    }

    map<string,bool> byl_tut;
    for(const auto& x: graph._vertexes){
        byl_tut[x.first] = false;
    }

    queue<string> fronta;
    map<string,set<string>> mda;
    fronta.push(lox);
    while(!fronta.empty()){
        string vertex = fronta.front();
        mda.insert(make_pair(vertex,set<string>()));
        fronta.pop();
        for(const auto& x: graph._vertexes[vertex]){
            if(mda[vertex].find(x) != mda[vertex].end()){
                return res;
            }
            mda[x].insert(vertex);
            for(const auto& y: mda[vertex]){
                mda[x].insert(y);
            }
            if(!byl_tut[x]){
                fronta.push(x);
                byl_tut[x] = true;
            }
        }

    }

    map<size_t,string> mda2;
    for(const auto& x: mda){
        auto it = mda2.insert(make_pair(x.second.size(), x.first));
        if(!it.second){
            return res;
        }
    }

    for(auto i = mda2.rbegin(); i != mda2.rend(); ++i){
        res.push_back(i->second);
    }
    return res;
}

template<typename M_>
template<typename Func>
bool CContest<M_>::isOrdered(Func func) const {
    list<string> list = getList(func);
    return !list.empty();
}

template<typename M_>
template <typename Func>
list<string> CContest<M_>::results(Func func) const{
    list<string> list = getList(func);
    if(list.empty()){
        throw logic_error("ne");
    }
    return list;
}

#ifndef __PROGTEST__
struct CMatch
{
public:
    CMatch                        ( int               a,
                                    int               b )
            : m_A ( a ),
              m_B ( b )
    {
    }

    int                      m_A;
    int                      m_B;
};

class HigherScoreThreshold
{
public:
    HigherScoreThreshold          ( int diffAtLeast )
            : m_DiffAtLeast ( diffAtLeast )
    {
    }
    int                      operator ()                   ( const CMatch & x ) const
    {
        return ( x . m_A > x . m_B + m_DiffAtLeast ) - ( x . m_B > x . m_A + m_DiffAtLeast );
    }
private:
    int            m_DiffAtLeast;
};

int                HigherScore                             ( const CMatch    & x )
{
    return ( x . m_A > x . m_B ) - ( x . m_B > x . m_A );
}

int                main                                    ( void )
{

    CContest<CMatch>  x;

    x       . addMatch ( "C++", "Pascal", CMatch ( 10, 3 ) )
            . addMatch ( "C++", "Java", CMatch ( 8, 1 ) )
            . addMatch ( "Pascal", "Basic", CMatch ( 40, 0 ) )
            . addMatch ( "Java", "PHP", CMatch ( 6, 2 ) )
            . addMatch ( "Java", "Pascal", CMatch ( 7, 3 ) )
            . addMatch ( "PHP", "Basic", CMatch ( 10, 0 ) )
            ;

    assert ( ! x . isOrdered ( HigherScore ) );
    try
    {
        list<string> res = x . results ( HigherScore );
        assert ( "Exception missing!" == nullptr );
    }
    catch ( const logic_error & e )
    {
    }
    catch ( ... )
    {
        assert ( "Invalid exception thrown!" == nullptr );
    }

    x . addMatch ( "PHP", "Pascal", CMatch ( 3, 6 ) );

    assert ( x . isOrdered ( HigherScore ) );
    try
    {
        list<string> res = x . results ( HigherScore );
        assert ( ( res == list <string>{ "C++", "Java", "Pascal", "PHP", "Basic" } ) );
    }
    catch ( ... )
    {
        assert ( "Unexpected exception!" == nullptr );
    }


    assert ( ! x . isOrdered ( HigherScoreThreshold ( 3 ) ) );
    try
    {
        list<string> res = x . results ( HigherScoreThreshold ( 3 ) );
        assert ( "Exception missing!" == nullptr );
    }
    catch ( const logic_error & e )
    {
    }
    catch ( ... )
    {
        assert ( "Invalid exception thrown!" == nullptr );
    }


    assert ( x . isOrdered ( [] ( const CMatch & x )
                             {
                                 return ( x . m_A < x . m_B ) - ( x . m_B < x . m_A );
                             } ) );
    try
    {
        list<string> res = x . results ( [] ( const CMatch & x )
                                         {
                                             return ( x . m_A < x . m_B ) - ( x . m_B < x . m_A );
                                         } );
        assert ( ( res == list<string>{ "Basic", "PHP", "Pascal", "Java", "C++" } ) );
    }
    catch ( ... )
    {
        assert ( "Unexpected exception!" == nullptr );
    }

    CContest<bool>  y;

    y . addMatch ( "Python", "PHP", true )
            . addMatch ( "PHP", "Perl", true )
            . addMatch ( "Perl", "Bash", true )
            . addMatch ( "Bash", "JavaScript", true )
            . addMatch ( "JavaScript", "VBScript", true );

    assert ( y . isOrdered ( [] ( bool v )
                             {
                                 return v ? 10 : - 10;
                             } ) );
    try
    {
        list<string> res = y . results ( [] ( bool v )
                                         {
                                             return v ? 10 : - 10;
                                         });
        assert ( ( res == list<string>{ "Python", "PHP", "Perl", "Bash", "JavaScript", "VBScript" } ) );
    }
    catch ( ... )
    {
        assert ( "Unexpected exception!" == nullptr );
    }

    y . addMatch ( "PHP", "JavaScript", false );
    assert ( !y . isOrdered ( [] ( bool v )
                              {
                                  return v ? 10 : - 10;
                              } ) );
    try
    {
        list<string> res = y . results ( [] ( bool v )
                                         {
                                             return v ? 10 : - 10;
                                         } );
        assert ( "Exception missing!" == nullptr );
    }
    catch ( const logic_error & e )
    {
    }
    catch ( ... )
    {
        assert ( "Invalid exception thrown!" == nullptr );
    }

    try
    {
        y . addMatch ( "PHP", "JavaScript", false );
        assert ( "Exception missing!" == nullptr );
    }
    catch ( const logic_error & e )
    {
    }
    catch ( ... )
    {
        assert ( "Invalid exception thrown!" == nullptr );
    }

    try
    {
        y . addMatch ( "JavaScript", "PHP", true );
        assert ( "Exception missing!" == nullptr );
    }
    catch ( const logic_error & e )
    {
    }
    catch ( ... )
    {
        assert ( "Invalid exception thrown!" == nullptr );
    }
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
