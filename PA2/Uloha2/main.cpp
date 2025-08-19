#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <memory>
using namespace std;
#endif /* __PROGTEST__ */

/** Class represents information about employee */
class Employee{
    friend class CPersonalAgenda;
    /** Function that compares to object of class Employee
     *  @param[in] left employee that need to be compared
     *  @param[in] right employee that need to be compared
     *  @return bool value, that is depended on comparing _name and _surname
     */
    friend bool cmpEmployee ( const Employee * left , const Employee * right );
public:
    /**Constructor initialize all class variables
     *  @param[in] name employee's name'
     *  @param[in] surname employee's surname
     *  @param[in] email employee's email
     *  @param[in] salary employee's salary
     */
    Employee ( const string & name, const string & surname , const string & email = "" , unsigned int salary = 0 );

private:
    /**@var contains employee's name*/
    string          _name;
    /**@var contains employee's surname*/
    string          _surname;
    /**@var contains employee's email*/
    string          _email;
    /**@var contains employee's salary*/
    unsigned int    _salary;
};

Employee::  Employee ( const string & name, const string & surname, const string & email , unsigned int salary )
    :   _name(name),
        _surname(surname),
        _email(email),
        _salary(salary)
{
}

bool cmpEmployee ( const Employee* left , const Employee * right){
    return left -> _surname != right -> _surname ? left -> _surname < right -> _surname : left -> _name < right -> _name;
}

/** Class represents storage for email and pointer
 *  This class helps to avoid duplicating data in database
 */
class CPair {
    friend class CPersonalAgenda;
public:
    /**Constructor initialize _email and _employeePtr
     *  @param[in] email employee's email'
     *  @param[in] ptr pointer to whom belongs email
     */
    CPair   ( const string & email , Employee * ptr = nullptr);
    /**Operator < for comparing two objects of class CPair
     * @param[in] right object for comparing
     * @return bool value, depending on comparing _email
     */
    bool operator< ( const CPair & right ) const;

private:
    /**@var Employee email*/
    string _email;
    /**@var pointer to whom this email belongs*/
    Employee * _employeePtr;
};

CPair::CPair ( const string & email, Employee * ptr )
    :   _email (email),
        _employeePtr(ptr)
{
}

bool CPair :: operator< ( const CPair & right ) const {
    return _email < right . _email;
}


/**Class represents database of employees*/
class CPersonalAgenda
{
public:
    /**Default constructor*/
    CPersonalAgenda  ( void ) = default;
    /**Destructor for deleting allocated memory */
    ~CPersonalAgenda ( void );
    /**
     * Method add new employee in database
     * @param[in] name employee's name
     * @param[in] surname employee's surname
     * @param[in] email employee's email
     * @param[in] salary employee's salary
     * @return true if new employee was added, otherwise false
     *         false is returned, when in database already exists employee with given name and surname, or with give email
     */
    bool add ( const string & name , const string & surname , const string & email , unsigned int salary );
    /**
     * Method del deletes employee from database with given name and surname
     * @param[in] name employee's name that need to be deleted
     * @param[in] surname employee's surname that need to be deleted
     * @return true if employee was deleted from database, otherwise false
     *         false is returned, when in database does not exist employee with given name and surname
     */
    bool del ( const string & name , const string & surname );
    /**
     * Method del deletes employee from database with given email
     * @param[in] email employee's email that need to be deleted
     * @return true if employee was deleted from database, otherwise false
     *         false is returned, when in database does not exist employee with given email
     */
    bool del ( const string & email );
    /**
     * Method changeName changes employee name and surname with given email
     * @param[in] email employee's email that name and surname need to be changed
     * @param[in] newName employee's new name
     * @param[in] newSurname employee's new surname
     * @return true if employee name and surname were changed, otherwise false
     *         false is returned, when in database does not exist employee with given email, or
     *         in database exists employee with given newName and newSurname
     */
    bool changeName ( const string & email , const string & newName , const string & newSurname );
    /**
     * Method changeEmail changes employee email with given name and surname
     * @param[in] name employee's name that need to be changed
     * @param[in] surname employee's surname that need to be changed
     * @param[in] newEmail employee's new email
     *
     * @return true if email was changed, otherwise false
     *         false is returned, when in database does not exist employee with given name and surname, or
     *         in database exists employee with given newEmail
     */
    bool changeEmail ( const string & name , const string & surname , const string & newEmail );
    /**
     * Method setSalary sets employee's salary with given name and surname
     * @param[in] name employee's name, whose salary need to be changed
     * @param[in] surname employee's surname, whose salary need to be changed
     * @param[in] salary employee's new salary
     * @return true if new salary was setted, otherwise false
     *         false is returned, when in database does not exist employee with given name and surname
     */
    bool setSalary ( const string & name , const string & surname, unsigned int salary );
    /**
     * Method setSalary sets employee's salary with given email
     * @param[in] email employee's email, whose salary need to be changed
     * @param[in] salary employee's new salary
     * @return true if email was changed, otherwise false
     *         false is returned, when in database does not exist employee with given email
     */
    bool setSalary ( const string & email , unsigned int salary );
    /**
     * Method getSalary gets employee's salary with given name and surname
     * @param[in] name employee's name, whose salary need to be returned
     * @param[in] surname employee's surname, whose salary need to be returned
     * @return employee's salary, if it exists in database
     *         0 is returned, when in database does not exist employee with given name and surname
     */
    unsigned int getSalary ( const string & name , const string & surname ) const;
    /**
    * Method setSalary sets employee's salary with given email
    * @param[in] email employee's email, whose salary need to be returned
    * @param[in] salary employee's new salary
    * @return employee's salary, if it exists in database
    *         0 is returned, when in database does not exist employee with given email
    */
    unsigned int  getSalary ( const string & email ) const;
    /**
     * Method getRank finds how well employee is paid in relation to others. The result is position
     * of the employee's salary from the lowest to the highest
     * @param[in] name employee's name, whose rank need to be determined
     * @param[in] surname employee's surname, whose rank need to be determined
     * @param[out] rankMin lowest rank of employee's salary
     * @param[out] rankMax highest rank of employee's salary
     * @return true, if rankMin and rankMax was determined
     *         false is returned, when in database does not exist employee with given name and surname,
     *         value of rankMin and rankMax don't change
     */
    bool getRank ( const string & name , const string & surname, int & rankMin, int & rankMax ) const;
    /**
     * Method getRank finds how well employee is paid in relation to others. The result is position
     * of the employee's salary from the lowest to the highest
     * @param[in] email employee's email, whose rank need to be determined
     * @param[out] rankMin lowest rank of employee's salary
     * @param[out] rankMax highest rank of employee's salary
     * @return true, if rankMin and rankMax was determined
     *         false is returned, when in database does not exist employee with given email,
     *         value of rankMin and rankMax don't change
     */
    bool getRank ( const string & email , int & rankMin , int & rankMax ) const;
    /**
     * Method getFirst finds first employee from database
     * @param[out] outName name of first employee from database
     * @param[out] outSurname surname of first employee from database
     * @return true, if name and surname was found
     *         false is returned, when in database does not exist any employee
     */
    bool getFirst ( string & outName , string & outSurname ) const;
    /**
     * Method getNext finds employee that goes after employee with given name and surname
     * @param[in] name employee's name, after whose need to be returned first employee
     * @param[in] surname employee's surname, after whose need to be returned first employee
     * @param[out] outName name of first employee after employee with given name and surname from database
     * @param[out] outSurname surname of first employee after employee with given name and surname from database
     * @return true, if name and surname was found
     *         false is returned, when in database does not exist employee with given name and surname,
     *         or employee with given name and surname is last in database
     */
    bool getNext ( const string & name , const string & surname , string & outName , string & outSurname ) const;

private:
    /**@var vector contains pointer to object of class Employee, that was added to database.
     * Vector is ordered by employee name and surname
     * */
    vector<Employee*> _employees;
    /**@var vector contains objects of CPair.
     *  Vector is ordered by employee email
     * */
    vector<CPair> _pairs;
    /**@var vector contains salaries of all employee
     * */
    vector<unsigned long> _salaries;

    /**
     * Method getByNames returns const_iterator to employee in database
     * @param[in] employee employee that need to be found
     * @returns const_iterator to employee in vector if database contains this employee
     *          otherwise vector.end()
     */
    vector<Employee*>::const_iterator getByNames ( const Employee & employee ) const;

    /**
     * Method findForChangeByNames returns iterator to employee in database, for changing, deleting,
     * or for correct inserting, without destroying order in vector
     * @param[in] employee employee that need to be found, or for correct insertion
     * @returns _iterator to employee in vector if database contains this employee or for correct insertion
     */
    vector<Employee*>::iterator findForChangeByNames (const Employee & employee );

    /**
     * Method getByEmail returns iterator pair in vector
     * @param[in] cpair pair that need to be found
     * @returns const_iterator to CPair in vector if database contains employee with mail from cpair
     *          otherwise vector.end()
     */
    vector<CPair>::const_iterator getByEmail ( const CPair & cpair ) const;

    /**
     * Method findForChangeByEmail returns pair in vector,  for changing, deleting,
     * or for correct inserting, without destroying order in vector
     * @param[in] cpair pair that need to be found
     * @returns iterator to CPair in vector if database contains employee with mail from cpair
     *          or for correct insertion
     */
    vector<CPair>::iterator findForChangeByEmail( const CPair & cpair );


};

vector<Employee*> :: const_iterator CPersonalAgenda :: getByNames ( const Employee & employee) const {
    auto res = lower_bound ( _employees.begin() , _employees.end()  , & employee, cmpEmployee );
    if ( res != _employees.end() && (*res) -> _surname == employee._surname && (*res) -> _name == employee . _name ) {
        return res;
    }
    return _employees.end();
}
vector<Employee*>::iterator CPersonalAgenda::findForChangeByNames ( const Employee & employee ) {
    auto res = lower_bound ( _employees.begin() , _employees.end() , & employee , cmpEmployee );
    return res;
}
vector<CPair>::const_iterator CPersonalAgenda::getByEmail ( const CPair & cpair ) const {
    auto res = lower_bound (_pairs . begin() , _pairs.end() , cpair);
    if ( res != _pairs.end() && res -> _email == cpair._email ) {
        return res;
    }
    return _pairs.end();
}
vector<CPair>::iterator CPersonalAgenda::findForChangeByEmail( const CPair & cpair ){
    auto res = lower_bound( _pairs.begin() , _pairs.end() , cpair);
    return res;
}

CPersonalAgenda::~CPersonalAgenda ( void ){
    for( auto i : _employees ) {
        delete i;
    }
}
bool CPersonalAgenda::add ( const string & name, const string & surname, const string & email, unsigned int salary ) {
    Employee* newEmp = new Employee ( name , surname , email , salary );
    CPair newPair ( email , newEmp );
    if ( getByNames( *newEmp ) != _employees.end() || getByEmail( newPair ) != _pairs.end() ) {
        delete newEmp;
        return false;
    }
    auto itEmp = findForChangeByNames( *newEmp );
    auto itPair = findForChangeByEmail( newPair );
    auto itSalary =  lower_bound( _salaries.begin() , _salaries.end() , salary );
    _employees.insert( itEmp , newEmp );
    _pairs.insert( itPair , newPair );
    _salaries.insert( itSalary , salary );
    return true;
}

bool CPersonalAgenda::del ( const string & name, const string & surname ) {
    if(getByNames(Employee(name,surname)) == _employees.end()) {
        return false;
    }
    auto itEmp = findForChangeByNames(Employee(name,surname));
    auto itPair = findForChangeByEmail(CPair((*itEmp)->_email));
    auto itSalary =  lower_bound(_salaries.begin(),_salaries.end(),(*itEmp)->_salary);
    delete itPair->_employeePtr;
    _pairs.erase( itPair );
    _employees.erase( itEmp );
    _salaries.erase( itSalary );
    return true;
}

bool CPersonalAgenda::del ( const string & email ) {
    if ( getByEmail( CPair( email ) ) == _pairs.end() ) {
        return false;
    }
    auto itPair = findForChangeByEmail( CPair( email ) );
    auto itEmp = findForChangeByNames( *(itPair -> _employeePtr) );
    auto itSalary =  lower_bound( _salaries.begin() , _salaries.end() , (*itEmp) -> _salary );
    delete itPair -> _employeePtr;
    _pairs.erase( itPair );
    _employees.erase( itEmp );
    _salaries.erase( itSalary );
    return true;
}

bool CPersonalAgenda::changeName ( const string & email , const string & newName , const string & newSurname ) {
    if  (      getByEmail( CPair(email) ) == _pairs.end()
            || getByNames(Employee( newName ,newSurname ) ) != _employees.end() ) {
        return false;
    }
    auto itPair = findForChangeByEmail( CPair( email ) );
    auto itOldEmp = findForChangeByNames(*(itPair -> _employeePtr) );
    _employees.erase( itOldEmp );

    Employee * newEmp = new Employee( newName , newSurname , email ,itPair -> _employeePtr -> _salary);
    auto itNewEmp = findForChangeByNames( *newEmp );
    delete itPair -> _employeePtr;
    itPair -> _employeePtr = newEmp;
    _employees.insert( itNewEmp , newEmp );

    return true;
}

bool CPersonalAgenda::changeEmail ( const string & name , const string & surname , const string & newEmail ) {
    auto itEmp = getByNames ( Employee ( name , surname ) );
    if ( getByEmail ( newEmail ) != _pairs.end() || itEmp == _employees.end() ) {
        return false;
    }
    auto itOldEmail = findForChangeByEmail ( (*itEmp)->_email );
    _pairs.erase( itOldEmail );
    CPair newPair( newEmail,*itEmp );
    auto itNewEmail = findForChangeByEmail ( newPair );
    _pairs.insert( itNewEmail , newPair );
    (*itEmp) -> _email = newEmail;

    return true;
}

bool CPersonalAgenda::setSalary ( const string & name , const string & surname , unsigned int salary ){
    Employee temp = Employee(name,surname);
    if(getByNames(temp) == _employees.end()){
        return false;
    }
    auto itEmployee = findForChangeByNames(temp);
    auto itOldSalary = lower_bound(_salaries.begin(),_salaries.end(),(*itEmployee)->_salary);
    (*itEmployee)->_salary = salary;
    _salaries.erase(itOldSalary);
    auto itNewSalary = lower_bound(_salaries.begin(),_salaries.end(),salary);
    _salaries.insert(itNewSalary,salary);

    return true;
}

bool CPersonalAgenda::setSalary ( const string & email, unsigned int salary ){
    CPair temp = CPair(email);
    if(getByEmail( CPair( temp ) ) == _pairs.end() ){
        return false;
    }
    auto itPair = findForChangeByEmail( CPair(temp) );
    auto itOldSalary = lower_bound( _salaries.begin() , _salaries.end() , (*itPair) ._employeePtr -> _salary);
    itPair -> _employeePtr -> _salary = salary;
    _salaries.erase( itOldSalary );
    auto itNewSalary = lower_bound( _salaries.begin() , _salaries.end() , salary );
    _salaries.insert( itNewSalary , salary );
    return true;
}

unsigned int CPersonalAgenda::getSalary ( const string & name, const string & surname ) const {
    Employee temp = Employee( name , surname );
    auto itEmployee = getByNames( temp );
    if ( itEmployee == _employees.end() ) {
        return 0;
    }
    return (*itEmployee) -> _salary;
}

unsigned int CPersonalAgenda::getSalary ( const string & email ) const {
    CPair temp = CPair( email );
    auto itPair = getByEmail( CPair( temp ) );
    if ( itPair == _pairs.end() ) {
        return 0;
    }
    return itPair -> _employeePtr -> _salary;
}

bool CPersonalAgenda::getRank ( const string & name , const string & surname , int & rankMin , int & rankMax ) const {
    Employee temp = Employee( name , surname );
    auto itEmployee = getByNames( temp );
    if ( itEmployee == _employees.end() ){
        return false;
    }
    auto itSalaryLow = lower_bound( _salaries.begin() , _salaries.end() , (*itEmployee) -> _salary );
    auto itSalaryUp = upper_bound( _salaries.begin() , _salaries.end() , (*itEmployee) -> _salary );
    rankMin = (int) distance( _salaries.begin() , itSalaryLow );
    rankMax = (int) distance( _salaries.begin() , itSalaryUp ) - 1;

    return true;
}

bool CPersonalAgenda::getRank( const string & email , int & rankMin , int & rankMax ) const {
    CPair temp = CPair( email );
    auto itPair = getByEmail( CPair( temp ) );
    if( itPair == _pairs.end() ) {
        return false;
    }
    auto itSalaryLow = lower_bound( _salaries.begin() , _salaries.end() , (itPair) -> _employeePtr -> _salary);
    auto itSalaryUp = upper_bound( _salaries.begin() , _salaries.end() , (itPair) -> _employeePtr -> _salary);
    rankMin = (int) distance( _salaries.begin() , itSalaryLow );
    rankMax = (int) distance( _salaries.begin() , itSalaryUp ) - 1;

    return true;
}

bool CPersonalAgenda::getFirst (string & outName, string & outSurname ) const {
    if ( _employees.empty() ){
        return false;
    }
    outName = ( *_employees.begin() ) -> _name;
    outSurname = ( *_employees.begin() ) -> _surname;
    return true;
}

bool CPersonalAgenda::getNext ( const string & name , const string & surname , string & outName, string & outSurname ) const {
    Employee temp = Employee( name , surname );
    auto itEmployee = getByNames( temp );
    if ( itEmployee == _employees.end() || ( itEmployee + 1 ) == _employees.end() ) {
        return false;
    }

    outName = ( *(itEmployee + 1) ) -> _name;
    outSurname = ( *(itEmployee + 1) ) -> _surname;
    return true;
}
