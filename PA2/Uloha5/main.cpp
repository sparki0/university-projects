#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iterator>
#include <functional>
using namespace std;

class CDate
{
public:
    //---------------------------------------------------------------------------------------------
    CDate                         ( int               y,
                                    int               m,
                                    int               d )
            : m_Year ( y ),
              m_Month ( m ),
              m_Day ( d )
    {
    }
    //---------------------------------------------------------------------------------------------
    int                      compare                       ( const CDate     & x ) const
    {
        if ( m_Year != x . m_Year )
            return m_Year - x . m_Year;
        if ( m_Month != x . m_Month )
            return m_Month - x . m_Month;
        return m_Day - x . m_Day;
    }
    //---------------------------------------------------------------------------------------------
    int                      year                          ( void ) const
    {
        return m_Year;
    }
    //---------------------------------------------------------------------------------------------
    int                      month                         ( void ) const
    {
        return m_Month;
    }
    //---------------------------------------------------------------------------------------------
    int                      day                           ( void ) const
    {
        return m_Day;
    }
    //---------------------------------------------------------------------------------------------
    friend ostream         & operator <<                   ( ostream         & os,
                                                             const CDate     & x )
    {
        char oldFill = os . fill ();
        return os << setfill ( '0' ) << setw ( 4 ) << x . m_Year << "-"
                  << setw ( 2 ) << static_cast<int> ( x . m_Month ) << "-"
                  << setw ( 2 ) << static_cast<int> ( x . m_Day )
                  << setfill ( oldFill );
    }
    //---------------------------------------------------------------------------------------------
private:
    int16_t                  m_Year;
    int8_t                   m_Month;
    int8_t                   m_Day;
};
#endif /* __PROGTEST__ */

class CInvoice
{
    friend class CVATRegister;
    friend class CSortOpt;
    friend bool equalLists ( const list<CInvoice> & a, const list<CInvoice> & b );
public:
    CInvoice(const CDate& date, const string& seller, const string& buyer, unsigned int amount, double vat );
    CInvoice(const CInvoice& src);
    CInvoice& operator=(const CInvoice& src);
    CDate date(void) const;
    string buyer(void) const;
    string seller(void) const;
    unsigned int amount(void) const;
    double vat(void) const;
    size_t ind(void) const;
    bool operator==(const CInvoice& right) const;
    bool operator!=(const CInvoice &right) const;
    bool operator<(const CInvoice& right) const;
    friend ostream & operator<<(ostream& out, const CInvoice& src){
        out << setw(10) << src.date() << setw(20) << src._seller << setw(40) << src._buyer
        << setw(40) << src._amount << setw(10) << src._vat;
        return out;
    }
private:
    size_t _ind;
    CDate _date;
    string _seller;
    string _buyer;
    string _normSeller;
    string _normBuyer;
    unsigned int _amount;
    double _vat;
};

CInvoice::CInvoice(const CDate &date, const string &seller, const string &buyer, unsigned int amount, double vat)
                    : _date(date), _seller(seller), _buyer(buyer), _amount(amount), _vat(vat) {

}
CInvoice::CInvoice(const CInvoice& src)
        :   _ind(src._ind) ,
            _date(src._date),
            _seller(src._seller),
            _buyer(src._buyer) ,
            _normSeller(src._normSeller),
            _normBuyer(src._normBuyer),
            _amount(src._amount),
            _vat(src._vat)
            {

            }

CInvoice& CInvoice::operator=(const CInvoice& src){
    if(this == &src){
        return *this;
    }
    _normSeller = src._normSeller;
    _normBuyer = src._normBuyer;
    _ind = src._ind;
    _date = src._date;
    _seller = src._seller;
    _buyer = src._buyer;
    _amount = src._amount;
    _vat = src._vat;
    return *this;
}
CDate CInvoice::date(void) const {
    return _date;
}

string CInvoice::buyer(void) const {
    return _buyer;
}

string CInvoice::seller(void) const {
    return _seller;
}

unsigned int CInvoice::amount(void) const {
    return _amount;
}

double CInvoice::vat(void) const {
    return _vat;
}

size_t CInvoice::ind(void) const{
    return _ind;
}

bool CInvoice::operator==(const CInvoice &right) const{
    return (!_date.compare(right._date)
                && _normBuyer == right._normBuyer
                && _normSeller == right._normSeller
                && _amount == right._amount
                && _vat == right._vat);
}

bool CInvoice::operator!=(const CInvoice &right) const{
    return !(*this == right);
}

bool CInvoice::operator<(const CInvoice& right) const{
    if(_date.compare(right._date))
        return ( _date.compare(right._date) < 0);
    if(_normSeller != right._normSeller)
        return _normSeller < right._normSeller;
    if(_normBuyer != right._normBuyer)
        return _normBuyer < right._normBuyer;
    if(_amount != right._amount)
        return _amount < right._amount;
    return _vat < right._vat;
}

class CSortOpt
{
    friend class CVATRegister;
public:
    static const int         BY_DATE                       = 0;
    static const int         BY_BUYER                      = 1;
    static const int         BY_SELLER                     = 2;
    static const int         BY_AMOUNT                     = 3;
    static const int         BY_VAT                        = 4;
    CSortOpt(void);
    CSortOpt& addKey(int key, bool ascending = true ) {_keys.push_back(pair<int,bool>(key,ascending)); return *this;};
    bool operator()(const CInvoice& a, const CInvoice& b) const;
private:
    vector<pair<int,bool>> _keys;
    string toLowerStr(const string& src) const{
        string res;
        for(auto& ch: src){
            res += (char)::tolower(ch);
        }
        return res;
    }
};

CSortOpt::CSortOpt(void) {

}

bool CSortOpt::operator()(const CInvoice &a, const CInvoice &b) const {
    for(const auto& i: _keys){
        switch(i.first){
            case BY_DATE:
                if(a.date().compare(b.date()))
                    return (a.date().compare(b.date()) < 0) == i.second;
                else { continue; }
            case BY_BUYER:
                if(toLowerStr(a._buyer) != toLowerStr(b._buyer)){
                    return (toLowerStr(a._buyer) < toLowerStr(b._buyer)) == i.second;
                }
                else { continue; }
            case BY_SELLER:
                if(toLowerStr(a._seller) != toLowerStr(b._seller)){
                    return (toLowerStr(a._seller) < toLowerStr(b._seller)) == i.second;
                }
                else { continue; }
            case BY_AMOUNT:
                if(a.amount() != b.amount())
                    return (a.amount() < b.amount()) == i.second;
                else { continue; }
            case BY_VAT:
                if(a.vat() != b.vat()) {
                    return (a.vat() < b.vat()) == i.second;
                }
                else { continue; }
        }
    }
    return a.ind() < b.ind();
}

class CVATRegister
{
    friend int main();
public:
    CVATRegister                  ( void ): _curInd(0) {};
    bool                     registerCompany               ( const string    & name );
    bool                     addIssued                     ( const CInvoice  & x );
    bool                     addAccepted                   ( const CInvoice  & x );
    bool                     delIssued                     ( const CInvoice  & x );
    bool                     delAccepted                   ( const CInvoice  & x );
    list<CInvoice>           unmatched                     ( const string    & company,
                                                             const CSortOpt  & sortBy ) const;
    void print() const{
        for(auto& i: _companies){
            cout << setw(20) << i.first << setw(40) << i.second << endl;
        }
    }
    void printIssued() const{
        for(auto& x : _issued){
            cout << x.first << endl;
            for(auto&y : x.second){
                for(auto& z : y.second){
                    cout << setw(10) << z._ind << z << endl;
                }
            }
        }
    }
    void printAccepted() const {
        for(auto& x : _accepted){
            cout << x.first << endl;
            for(auto&y : x.second){
                for(auto& z : y.second){
                    cout << setw(5) << endl << z._ind << z << endl;
                }
            }
        }
    }
private:
    size_t _curInd;
    std::map<string, string> _companies;
    std::map<string, map<string,set<CInvoice>>> _issued;
    std::map<string, map<string,set<CInvoice>>> _accepted;
    string toNormStr(const string& src) const;
};

string CVATRegister::toNormStr(const string& src) const{
    string res;
    unique_copy(src.begin(),src.end(), back_inserter(res),
           [] (char a, char b) {return isspace(a) && isspace(b);});
    if(res[0] == ' '){
        res = res.substr(1, res.length() - 1);
    }
    if(res[res.length() - 1] == ' '){
        res = res.substr(0, res.length() - 1);
    }
    for(size_t i = 0; i < res.length(); ++i){
        res[i] = (char)tolower(res[i]);
    }

    return res;
}

bool CVATRegister::registerCompany(const string &name) {
    string newName = toNormStr(name);
    if(_companies.find(newName) != _companies.end() && _companies.find(newName)->first == newName){
        return false;
    }
    _companies[newName] = name;
    return true;
}

bool CVATRegister::addIssued(const CInvoice &x) {
    string sellerName = toNormStr(x._seller);
    string buyerName = toNormStr(x._buyer);
    if(_companies.find(sellerName) == _companies.end() || _companies.find(buyerName) == _companies.end() || sellerName == buyerName){
        return false;
    }
    CInvoice temp(x);

    temp._normSeller = sellerName;
    temp._normBuyer = buyerName;
    auto ptr = _issued[sellerName][buyerName].find(temp);
    if(ptr != _issued[sellerName][buyerName].end() && (*ptr) == temp){
        return false;
    }
    temp._ind = _curInd++;
    temp._seller = _companies[sellerName];
    temp._buyer = _companies[buyerName];
    _issued[sellerName][buyerName].insert(temp);
    return true;
}

bool CVATRegister::addAccepted(const CInvoice &x) {
    string sellerName = toNormStr(x._seller);
    string buyerName = toNormStr(x._buyer);
    if(_companies.find(sellerName) == _companies.end() || _companies.find(buyerName) == _companies.end() || sellerName == buyerName){
        return false;
    }
    CInvoice temp(x);

    temp._normSeller = sellerName;
    temp._normBuyer = buyerName;
    auto ptr = _accepted[buyerName][sellerName].find(temp);
    if(ptr != _accepted[buyerName][sellerName].end() && (*ptr) == temp){
        return false;
    }
    temp._ind = _curInd++;
    temp._seller = _companies[sellerName];
    temp._buyer = _companies[buyerName];
    _accepted[buyerName][sellerName].insert(temp);
    return true;
}

bool CVATRegister::delIssued(const CInvoice &x) {
    string sellerName = toNormStr(x._seller);
    string buyerName = toNormStr(x._buyer);
    if(_companies.find(sellerName) == _companies.end() || _companies.find(buyerName) == _companies.end() || sellerName == buyerName){
        return false;
    }
    auto ptrIss = _issued.find(sellerName);
    if(ptrIss == _issued.end() || (ptrIss != _issued.end() && ptrIss->first != sellerName)){
        return false;
    }
    auto toDel = ptrIss->second.find(buyerName);
    if(toDel == ptrIss->second.end() || (toDel != ptrIss->second.end() && toDel->first != buyerName)){
        return false;
    }
    CInvoice temp = x;
    temp._normBuyer = buyerName;
    temp._normSeller = sellerName;
    auto tochno = toDel->second.find(temp);
    if(tochno == toDel->second.end() || (tochno != toDel->second.end() && (*tochno) != temp)){
        return false;
    }
    toDel->second.erase(temp);
    return true;
}

bool CVATRegister::delAccepted(const CInvoice &x) {
    string sellerName = toNormStr(x._seller);
    string buyerName = toNormStr(x._buyer);
    if(_companies.find(sellerName) == _companies.end() || _companies.find(buyerName) == _companies.end() || sellerName == buyerName){
        return false;
    }
    auto ptrAcc = _accepted.find(buyerName);
    if(ptrAcc == _accepted.end() || (ptrAcc != _accepted.end() && ptrAcc->first != buyerName)){
        return false;
    }
    auto toDel = ptrAcc->second.find(sellerName);
    if(toDel == ptrAcc->second.end() || (toDel != ptrAcc->second.end() && toDel->first != sellerName)){
        return false;
    }
    CInvoice temp = x;
    temp._normBuyer = buyerName;
    temp._normSeller = sellerName;
    auto tochno = toDel->second.find(temp);
    if(tochno == toDel->second.end() || (tochno != toDel->second.end() && (*tochno) != temp)){
        return false;
    }
    toDel->second.erase(temp);

    return true;
}

list<CInvoice> CVATRegister::unmatched(const string &company, const CSortOpt &sortBy) const {
    list<CInvoice> res;
    string name = toNormStr(company);
    if(_companies.find(name) == _companies.end()){
        return res;
    }
    set<CInvoice> resSet;
   auto ptrIss = _issued.find(name);
    if(ptrIss != _issued.end() && ptrIss->first == name){
        for(const auto& i: ptrIss->second){
            for(const auto& j: i.second){
                resSet.insert(j);
            }
        }
    }
    for(const auto& i: _issued){
        auto ptrIs1 = i.second.find(name);
        if(ptrIs1 != i.second.end() && ptrIs1->first == name){
            for(const auto& j: ptrIs1 ->second){
                resSet.insert(j);
            }
        }

    }
    for(const auto& i: _accepted){
        auto ptrAcc = i.second.find(name);
        if(ptrAcc != i.second.end() && ptrAcc->first == name ){
            for(const auto& j: ptrAcc->second){
                auto ptrFind = resSet.find(j);
                if(ptrFind != resSet.end() && (*ptrFind) == j){
                    resSet.erase(j);
                } else {
                    resSet.insert(j);
                }
            }
        }
    }

    auto ptrAcc = _accepted.find(name);
    if(ptrAcc != _accepted.end() && ptrAcc->first == name){
        for(const auto& i: ptrAcc->second){
            for(const auto& j: i.second){
                auto ptrFind = resSet.find(j);
                if(ptrFind != resSet.end() && (*ptrFind) == j){
                    resSet.erase(j);
                } else {
                    resSet.insert(j);
                }
            }
        }
    }

    for(auto& i: resSet){
        res.push_back(i);
    }
//    for(auto& x: res){
//        cout << x << endl;
//    }

//    cout << endl;
    res.sort(sortBy);
//    for(auto& x: res){
//        cout << x << endl;
//    }



    return res;
}
