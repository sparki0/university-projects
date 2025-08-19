#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <ctime>
#include <climits>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <memory>
using namespace std;
#endif /* __PROGTEST__ */

// uncomment if your code implements initializer lists
 #define EXTENDED_SYNTAX
class CRangeList;
class CRange
{
    friend ostream& operator<<(ostream& out, const CRange& right){
        auto flags = out.flags();
        out.unsetf(flags);
        if(right._min == right._max){
            out << right._min;
        }
        else{
            out << '<' << right._min << ".." << right._max << '>';
        }
        out.setf(flags);
        return out;
    }
    friend class CRangeList;
    friend ostream& operator<<(ostream& out, const CRangeList& src);
public:
    CRange(long long min, long long max);
    CRange(const CRange& org): _min(org._min), _max(org._max) {}
    CRangeList operator+(const CRange& right) const;
    CRangeList operator-(const CRange& right) const;
    bool operator==(const CRange& right) const;
    bool operator!=(const CRange& right) const;
    bool operator<(const CRange& right) const;

private:
    long long _min;
    long long _max;
};

class CRangeList
{
public:

    // constructor
    CRangeList() {};
    CRangeList(const initializer_list<CRange>& src);
    // includes long long / range
    bool includes(long long num) const;
    bool includes(const CRange& range) const;
    // += range / range list
    CRangeList operator+(const CRange& right) const;
    CRangeList& operator+=(const CRange& right);
    CRangeList& operator+=(const CRangeList& right);
    // -= range / range list
    CRangeList operator-(const CRange& right) const;
    CRangeList& operator-=(const CRange& right);
    CRangeList& operator-=(const CRangeList& right);

    // = range / range list
    CRangeList& operator=(const CRange& right);
    CRangeList& operator=(const CRangeList& right);
    // operator ==
    bool operator==(const CRangeList& right) const;
    // operator !=
    bool operator!=(const CRangeList& right) const;
    // operator <<
    friend ostream& operator<<(ostream& out, const CRangeList& src);
    vector<CRange>::iterator begin();
    vector<CRange>::iterator end();
private:
    vector<CRange> _ranges;

};

//Crange
CRange::CRange(long long min, long long max){
    if(min > max){
        throw logic_error("");
    }
    _min = min;
    _max = max;
}
bool CRange::operator<(const CRange& right) const{
    return (_min < right._min);
}
CRangeList CRange::operator+(const CRange& right) const{
    CRangeList res;
    res += *this;
    res += right;
    return res;
}

CRangeList CRange::operator-(const CRange& right) const{
    CRangeList res;
    res += *this;
    res -= right;
    return res;
}

bool CRange::operator==(const CRange& right) const{
    return (_min == right._min && _max == right._max);
}
bool CRange::operator!=(const CRange& right) const{
    return !(*this == right);
}

//Crange

//CrangeList
CRangeList& CRangeList::operator+=(const CRange& right){
    if(_ranges.empty()){
        _ranges.push_back(right);
        return *this;
    }
    auto it = lower_bound(_ranges.begin(),_ranges.end(),right);
    it = _ranges.insert(it,right);
    auto jt = it;
    if(jt != _ranges.end()) {
        auto kt = jt + 1;
        for (; kt != _ranges.end() && jt->_max >= ((kt->_min == LLONG_MIN) ? (kt->_min) : (kt->_min - 1));) {
            jt->_max = jt->_max > kt->_max ? jt->_max : kt->_max;
            _ranges.erase(kt);
        }
    }
    if(it != _ranges.begin()){
        jt = it - 1;
        if(jt->_max >= ((it->_min == LLONG_MIN) ? (it->_min) : (it->_min - 1))){
            it->_min = jt->_min;
            it->_max = it->_max > jt->_max ? it->_max : jt->_max;
            _ranges.erase(jt);
        }
    }

    return *this;
}
CRangeList& CRangeList::operator+=(const CRangeList& right){
    for(const auto& i: right._ranges){
        *this += i;
    }
    return *this;
}
CRangeList CRangeList::operator+(const CRange& right) const{
    CRangeList res = *this;
    res += right;
    return res;
}

CRangeList CRangeList::operator-(const CRange& right) const{
    CRangeList res = *this;
    res -= right;
    return res;
}

CRangeList& CRangeList::operator-=(const CRange& right){
    if(_ranges.empty()){
        return *this;
    }
    CRange temp(right);
    if(temp._min < _ranges.begin()->_min && temp._max > (_ranges.end() - 1)->_max){
        _ranges.clear();
        return *this;
    }


    for(auto it = _ranges.begin(); it != _ranges.end(); ++it){
        if(temp._min > it->_min && temp._max < it->_max){
            CRange newR(temp._max + 1, it->_max);
            it->_max = temp._min - 1;
            _ranges.insert(it+1,newR);
            break;
        }
        else if(temp._min == it->_min && temp._max < it->_max){
            it->_min = temp._max + 1;
            break;
        }
        else if(temp._min > it->_min && temp._max == it->_max){
            it->_max = temp._min - 1;
            break;
        }
        else if(temp._min <= it->_min && temp._max >= it->_max){
            _ranges.erase(it);
            --it;
        }
        else if(temp._min > it->_min && temp._min <= it->_max && temp._max >= it->_max){
            it->_max = temp._min - 1;
        }
        else if(temp._max < it->_max && temp._max >= it->_min && temp._min <= it->_min){
            it->_min = temp._max+ 1;
        }
    }
    return *this;
}

bool CRangeList::operator==(const CRangeList& right) const{
    if(_ranges.size() != right._ranges.size()){
        return false;
    }
    for(int i = 0; i < (int)_ranges.size(); ++i){
        if(_ranges[i] != right._ranges[i]){
            return false;
        }
    }
    return true;
}

bool CRangeList::operator!=(const CRangeList& right) const{
    return !(*this == right);
}

CRangeList& CRangeList::operator-=(const CRangeList& right){
    for(const auto& i: right._ranges){
        *this -= i;
    }
    return *this;
}


CRangeList& CRangeList::operator=(const CRange& right){
    _ranges.clear();
    _ranges.push_back(right);
    return *this;
}
CRangeList& CRangeList::operator=(const CRangeList& right){
    if(this != &right){
        _ranges.clear();
        for(const auto& i: right._ranges){
            _ranges.push_back(i);
        }
    }
    return *this;
}

ostream& operator<<(ostream &out, const CRangeList &src) {
    auto flags = out.flags();
    out.unsetf(flags);
    out << '{';
    for(int i = 0; i < (int)src._ranges.size(); ++i){
        out << src._ranges[i];
        if(i != (int)src._ranges.size() - 1)
            out << ',';
    }
    out << '}';
    out.setf(flags);
    return out;
}

bool CRangeList::includes(long long num) const{
    if(_ranges.empty()){
        return false;
    }
    CRange temp(num,num);
    auto it = lower_bound(_ranges.begin(),_ranges.end(),temp);
    if(it != _ranges.begin() && it->_min != num){
        --it;
    }
    return (num >= it->_min && num <= it->_max);
}
bool CRangeList::includes(const CRange& range) const{
    if(_ranges.empty()){
        return false;
    }
    auto it = lower_bound(_ranges.begin(),_ranges.end(),range);
    if((it != _ranges.begin() && it->_min != range._min)){
        --it;
    }
    return (range._min >= it->_min && range._max <= it->_max);
}

CRangeList::CRangeList(const initializer_list<CRange> &src) {
    for(auto& i: src){
        *this += i;
    }
}

vector<CRange>::iterator CRangeList::begin() {
    return _ranges.begin();
}

vector<CRange>::iterator CRangeList::end() {
    return _ranges.end();
}
