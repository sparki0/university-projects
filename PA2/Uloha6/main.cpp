#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <typeinfo>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>
#include <type_traits>
using namespace std;

class CRect
{
public:
    CRect                         ( double            x,
                                    double            y,
                                    double            w,
                                    double            h )
            : m_X ( x ),
              m_Y ( y ),
              m_W ( w ),
              m_H ( h )
    {
    }
    friend ostream         & operator <<                   ( ostream         & os,
                                                             const CRect     & x )
    {
        return os << '(' << x . m_X << ',' << x . m_Y << ',' << x . m_W << ',' << x . m_H << ')';
    }
    double                   m_X;
    double                   m_Y;
    double                   m_W;
    double                   m_H;
};
#endif /* __PROGTEST__ */

class CItem{
public:
    CItem(int id, const CRect& relPos) : _id(id), _relPos(relPos), _absPos(0,0,0,0) {}
    virtual ~CItem () = default;

    friend ostream& operator<<(ostream& out, const CItem& item);

    virtual void print(ostream& out, int indent, bool isCBoxLast = false) const = 0;

    virtual CItem* clone() const = 0;

    int getId() const { return _id; };

    CRect getRelPos() const { return _relPos; };

    void setAbsPos(const CRect &windowPos) {
        _absPos.m_X = windowPos.m_W * _relPos.m_X + windowPos.m_X;
        _absPos.m_Y = windowPos.m_H * _relPos.m_Y + windowPos.m_Y;
        _absPos.m_W = windowPos.m_W * _relPos.m_W;
        _absPos.m_H = windowPos.m_H * _relPos.m_H;
    };
protected:
    int _id;
    CRect _relPos;
    CRect _absPos;
};

ostream &operator<<(ostream &out, const CItem &item) {
    item.print(out,0);
    return out;
}

class CItemNamed: public  CItem{
public:
    CItemNamed(int id, const CRect& relPos, const string& name)
            : CItem(id,relPos), _name(name)
    {}
protected:
    string _name;
};

class CWindow: public CItemNamed
{
public:
    CWindow(int id, const string& title, const CRect& absPos )
            : CItemNamed(id,CRect(0,0,1,1),title)
    {
        setAbsPos(absPos);
    }
    // add
    CWindow& add(const CItem& item){
        auto it = item.clone();
        it->setAbsPos(_absPos);
        _vector.emplace_back(it);
        return *this;
    }
    // search
    CItem* search(int id) const {
        for(auto it = _vector.begin(); it != _vector.end(); ++it){
            if(it->get()->getId() == id){
                return it->get();
            }
        }
        return nullptr;
    }
    // setPosition
    void setPosition(const CRect& newPos){
        _absPos = newPos;
        for(auto& x: _vector){
            x->setAbsPos(_absPos);
        }
    }

    CWindow(const CWindow& src)
            : CItemNamed(src._id,CRect(0,0,1,1),src._name)
    {
        setAbsPos(src._absPos);
        for(auto& x: src._vector){
            add(*x);
        }
    }


    CWindow& operator=(const CWindow& src) {
        if (this == &src) {
            return *this;
        }
        _vector.clear();
        CWindow temp(src);
        _id = temp._id;
        _name = temp._name;
        _relPos = temp._relPos;
        _absPos = temp._absPos;
        swap(_vector,temp._vector);
        return *this;
    }

    CItem* clone() const override{
        return new CWindow(*this);
    }
    void print(ostream& out, int indent, bool isCBoxLast = false) const override{
        out << string(indent,' ') << '[' << _id << "] " << "Window \""
            << _name << "\" " << _absPos << '\n';
        for(auto it = _vector.begin(); it != _vector.end(); ++it){
            auto jt = it;
            jt++;
            out << "+-";
            it->get()->print(out,1,jt == _vector.end());
        }
    }
private:
    vector<unique_ptr<CItem>> _vector;
};

class CButton: public CItemNamed
{
public:
    CButton(int id, const CRect& relPos, const string& name)
            : CItemNamed(id,relPos,name)  {};

    CItem* clone() const override{
        return new CButton(*this);
    }
    void print(ostream& out, int indent, bool isCBoxLast = false) const override{
        out << string(indent,' ') << '[' << _id << "] " << "Button \""
            << _name << "\" " << _absPos << '\n';
    }
};

class CInput: public CItemNamed
{
public:
    CInput(int id, const CRect& relPos, const string& value )
            : CItemNamed(id,relPos,value)
    {
    }
    // setValue
    void setValue(const string& newValue){
        _name = newValue;
    }
    string getValue(){
        return _name;
    }

    CItem* clone() const override{
        return new CInput(*this);
    }
    void print(ostream& out, int indent, bool isCBoxLast = false) const override{
        out << string(indent,' ') << '[' << _id << "] " << "Input \""
            << _name << "\" " << _absPos << '\n';
    }
};


class CLabel: public CItemNamed
{
public:
    CLabel(int id, const CRect& relPos, const string& label )
            : CItemNamed(id,relPos,label)
    {
    };

    CItem* clone() const override{
        return new CLabel(*this);
    }
    void print(ostream& out, int indent, bool isCBoxLast = false) const override{
        out << string(indent,' ') << '[' << _id << "] " << "Label \""
            << _name << "\" " << _absPos << '\n';
    }
};
class CComboBox: public CItem
{
public:
    CComboBox(int id, const CRect& relPos )
            : CItem(id,relPos), _selectedIdx(0)
    {
    }
    // add
    CComboBox& add(const string& value){
        _items.push_back(value);
        return *this;
    }
    // setSelected
    void setSelected(size_t idx){
        if(idx >= _items.size()){
            throw out_of_range("Out of range");
        }
        _selectedIdx = idx;
    }
    // getSelected
    size_t getSelected() const{
        return _selectedIdx;
    }

    CItem* clone() const override{
        return new CComboBox(*this);
    }

    void print(ostream& out, int indent, bool isCBoxLast = false) const override{
        out << string(indent,' ') << '[' << _id << "] " << "ComboBox " << _absPos << '\n';
        for(size_t i = 0; i < _items.size(); ++i){
            if(indent != 0){
                out << (isCBoxLast ? "   " : "|  ");
            }
            out << "+-";
            if(_selectedIdx == i) {
                out << '>' << _items[i] << "<\n";
            } else {
                out << ' ' << _items[i] << "\n";
            }
        }
    }
private:
    size_t _selectedIdx;
    vector<string> _items;

};
// output operators

#ifndef __PROGTEST__
template <typename _T>
string toString ( const _T & x )
{
    ostringstream oss;
    oss << x;
    return oss . str ();
}

int main ( void )
{
    assert ( sizeof ( CButton ) - sizeof ( string ) < sizeof ( CComboBox ) - sizeof ( vector<string> ) );
    assert ( sizeof ( CInput ) - sizeof ( string ) < sizeof ( CComboBox ) - sizeof ( vector<string> ) );
    assert ( sizeof ( CLabel ) - sizeof ( string ) < sizeof ( CComboBox ) - sizeof ( vector<string> ) );
    CWindow a ( 0, "Sample window", CRect ( 10, 10, 600, 480 ) );
    a . add ( CButton ( 1, CRect ( 0.1, 0.8, 0.3, 0.1 ), "Ok" ) ) . add ( CButton ( 2, CRect ( 0.6, 0.8, 0.3, 0.1 ), "Cancel" ) );
    a . add ( CLabel ( 10, CRect ( 0.1, 0.1, 0.2, 0.1 ), "Username:" ) );
    a . add ( CInput ( 11, CRect ( 0.4, 0.1, 0.5, 0.1 ), "chucknorris" ) );
    a . add ( CComboBox ( 20, CRect ( 0.1, 0.3, 0.8, 0.1 ) ) . add ( "Karate" ) . add ( "Judo" ) . add ( "Box" ) . add ( "Progtest" ) );
    assert ( toString ( a ) ==
             "[0] Window \"Sample window\" (10,10,600,480)\n"
             "+- [1] Button \"Ok\" (70,394,180,48)\n"
             "+- [2] Button \"Cancel\" (370,394,180,48)\n"
             "+- [10] Label \"Username:\" (70,58,120,48)\n"
             "+- [11] Input \"chucknorris\" (250,58,300,48)\n"
             "+- [20] ComboBox (70,154,480,48)\n"
             "   +->Karate<\n"
             "   +- Judo\n"
             "   +- Box\n"
             "   +- Progtest\n" );
    CWindow b = a;
    assert ( toString ( *b . search ( 20 ) ) ==
             "[20] ComboBox (70,154,480,48)\n"
             "+->Karate<\n"
             "+- Judo\n"
             "+- Box\n"
             "+- Progtest\n" );
    assert ( dynamic_cast<CComboBox &> ( *b . search ( 20 ) ) . getSelected () == 0 );
    dynamic_cast<CComboBox &> ( *b . search ( 20 ) ) . setSelected ( 3 );
    assert ( dynamic_cast<CInput &> ( *b . search ( 11 ) ) . getValue () == "chucknorris" );
    dynamic_cast<CInput &> ( *b . search ( 11 ) ) . setValue ( "chucknorris@fit.cvut.cz" );
    b . add ( CComboBox ( 21, CRect ( 0.1, 0.5, 0.8, 0.1 ) ) . add ( "PA2" ) . add ( "OSY" ) . add ( "Both" ) );
    assert ( toString ( b ) ==
             "[0] Window \"Sample window\" (10,10,600,480)\n"
             "+- [1] Button \"Ok\" (70,394,180,48)\n"
             "+- [2] Button \"Cancel\" (370,394,180,48)\n"
             "+- [10] Label \"Username:\" (70,58,120,48)\n"
             "+- [11] Input \"chucknorris@fit.cvut.cz\" (250,58,300,48)\n"
             "+- [20] ComboBox (70,154,480,48)\n"
             "|  +- Karate\n"
             "|  +- Judo\n"
             "|  +- Box\n"
             "|  +->Progtest<\n"
             "+- [21] ComboBox (70,250,480,48)\n"
             "   +->PA2<\n"
             "   +- OSY\n"
             "   +- Both\n" );
    assert ( toString ( a ) ==
             "[0] Window \"Sample window\" (10,10,600,480)\n"
             "+- [1] Button \"Ok\" (70,394,180,48)\n"
             "+- [2] Button \"Cancel\" (370,394,180,48)\n"
             "+- [10] Label \"Username:\" (70,58,120,48)\n"
             "+- [11] Input \"chucknorris\" (250,58,300,48)\n"
             "+- [20] ComboBox (70,154,480,48)\n"
             "   +->Karate<\n"
             "   +- Judo\n"
             "   +- Box\n"
             "   +- Progtest\n" );
    b . setPosition ( CRect ( 20, 30, 640, 520 ) );
    assert ( toString ( b ) ==
             "[0] Window \"Sample window\" (20,30,640,520)\n"
             "+- [1] Button \"Ok\" (84,446,192,52)\n"
             "+- [2] Button \"Cancel\" (404,446,192,52)\n"
             "+- [10] Label \"Username:\" (84,82,128,52)\n"
             "+- [11] Input \"chucknorris@fit.cvut.cz\" (276,82,320,52)\n"
             "+- [20] ComboBox (84,186,512,52)\n"
             "|  +- Karate\n"
             "|  +- Judo\n"
             "|  +- Box\n"
             "|  +->Progtest<\n"
             "+- [21] ComboBox (84,290,512,52)\n"
             "   +->PA2<\n"
             "   +- OSY\n"
             "   +- Both\n" );
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
