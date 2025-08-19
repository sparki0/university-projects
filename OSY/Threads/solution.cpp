#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>
#include <array>
#include <iterator>
#include <set>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <compare>
#include <queue>
#include <stack>
#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <condition_variable>
#include <pthread.h>
#include <semaphore.h>
#include "progtest_solver.h"
#include "sample_tester.h"

using namespace std;
#endif /* __PROGTEST__ */

enum ProblemType{ MIN, CNT };

class MiniProblem{
public:
    MiniProblem(size_t firmId, size_t packId, ProblemType type, APolygon polygon)
            : _firmId(firmId), _packId(packId), _type(type), _polygon(std::move(polygon))
    {}

    size_t getFirmId() { return _firmId; }
    size_t getPackId() { return _packId; }
    ProblemType getProbType() { return _type; }
    APolygon getPolygon() { return _polygon; }
private:
    size_t _firmId;
    size_t _packId;
    ProblemType _type;
    APolygon _polygon;
};

using AMiniProblem = std::shared_ptr<MiniProblem>;

class FullPack{
public:
    class ComparatorPack{
    public:
        bool operator()(const shared_ptr<FullPack> left, const shared_ptr<FullPack> right) { return left->_packId > right->_packId; }
    };
    FullPack(size_t packId, AProblemPack problemPack) :
            _packId(packId), _currMinInd(0), _currCntInd(0),
            _minFinished(false), _cntFinished(false), _problemPack(std::move(problemPack))
    {}
    void incMinInd() {
        if(_problemPack->m_ProblemsMin.empty()) {
            _minFinished = true;
            return;
        }
        ++_currMinInd;
        if(_currMinInd == _problemPack->m_ProblemsMin.size()) {
            _minFinished = true;
        }
    }

    void incCntInd() {
        if(_problemPack->m_ProblemsCnt.empty()) {
            _cntFinished = true;
            return;
        }
        ++_currCntInd;
        if(_currCntInd == _problemPack->m_ProblemsCnt.size()){
            _cntFinished = true;
        }
    }

    size_t getPackId() { return _packId; }
    bool isFinished() { return _minFinished && _cntFinished; }
    AProblemPack getPack() { return _problemPack; }
private:
    size_t _packId;
    size_t _currMinInd;
    size_t _currCntInd;
    bool _minFinished;
    bool _cntFinished;
    AProblemPack _problemPack;
};

using AFullPack = std::shared_ptr<FullPack>;

class Buffer{
public:
    void add(AMiniProblem q) {
        unique_lock ul(_mtx);
        _buffer.emplace(std::move(q));
    }

    AMiniProblem front() {
        unique_lock ul(_mtx);
        return _buffer.front();
    }

    void pop() {
        unique_lock ul(_mtx);
        _buffer.pop();
    }

    bool empty() {
        unique_lock ul(_mtx);
        return _buffer.empty();
    }
private:
    mutex _mtx;
    queue<AMiniProblem> _buffer;
};

class Firm{
public:
    Firm(size_t firmId, ACompany company, Buffer & buffer, condition_variable & cvWork, atomic<bool> & finishedWork);
    void incProblem(AMiniProblem problem);
    void createCommThrs();

    void stopReceiver() { _receiverThr.join() ; }
    void stopDeliver() {
        _cvFirm.notify_one();
        _deliverThr.join();
    }
private:
    void receiverThrWork();
    void deliverThrWork();

    size_t _firmId;
    size_t _currPackId = 0;
    ACompany _company;
    Buffer & _buffer;
    map<size_t, AFullPack> _problemMap;

    thread _receiverThr;
    thread _deliverThr;

    condition_variable & _cvWorkQ;
    atomic<bool> & _at_workingFinished;

    mutex _mtxPack;
    mutex _mtxFirm;
    condition_variable _cvFirm;
    atomic<bool> _at_receiveFinished = false;

    priority_queue<AFullPack, vector<AFullPack>, FullPack::ComparatorPack> _solvedQ;
};
Firm::Firm(size_t firmId, ACompany company, Buffer & buffer, condition_variable & cvWork, atomic<bool> & finishedWork)
        :   _firmId(firmId),
            _currPackId(0),
            _company(std::move(company)),
            _buffer(buffer),
            _cvWorkQ(cvWork),
            _at_workingFinished(finishedWork),
            _at_receiveFinished(true)
{
}

void Firm::incProblem(AMiniProblem problem) {
    unique_lock mapLock(_mtxPack);
    if(problem->getProbType() == ProblemType::MIN) {
        _problemMap[problem->getPackId()]->incMinInd();
    }
    else {
        _problemMap[problem->getPackId()]->incCntInd();
    }

    if(_problemMap[problem->getPackId()]->isFinished()) {
        auto solvedP = _problemMap[problem->getPackId()];
        _problemMap.erase(problem->getPackId());
        mapLock.unlock();

        unique_lock firmLock(_mtxFirm);
        _solvedQ.emplace(solvedP);
        firmLock.unlock();
        _cvFirm.notify_one();
    }
}

void Firm::createCommThrs() {
    _receiverThr = thread(&Firm::receiverThrWork, this);
    _deliverThr = thread(&Firm::deliverThrWork, this);
}

void Firm::receiverThrWork() {
    size_t packId = 0;
    while(true) {
        auto newProblem = _company->waitForPack();
        if(!newProblem.get()) {
            _at_receiveFinished = true;
            _cvFirm.notify_one();
            return;
        }
        unique_lock packLock(_mtxPack);
        _problemMap[packId] = make_shared<FullPack>(packId,newProblem);
        packLock.unlock();

        for(auto minPtr : newProblem->m_ProblemsMin) {
            _buffer.add(make_shared<MiniProblem>(_firmId, packId, ProblemType::MIN, minPtr));
        }
        if(newProblem->m_ProblemsMin.empty()) {
            _buffer.add(make_shared<MiniProblem>(_firmId, packId, ProblemType::MIN, nullptr));
        }

        for(auto cntPtr : newProblem->m_ProblemsCnt) {
            _buffer.add(make_shared<MiniProblem>(_firmId, packId, ProblemType::CNT, cntPtr));
        }
        if(newProblem->m_ProblemsCnt.empty()) {
            _buffer.add(make_shared<MiniProblem>(_firmId, packId, ProblemType::CNT, nullptr));
        }
        ++packId;
        _cvWorkQ.notify_one();
    }
}

void Firm::deliverThrWork() {
    while(true) {
        unique_lock firmLock(_mtxFirm);
        _cvFirm.wait(firmLock, [&] { return ( !_solvedQ.empty() && _solvedQ.top()->getPackId() == _currPackId ) || (_at_receiveFinished && _at_workingFinished) ;});
        if(_solvedQ.empty() && _at_receiveFinished && _at_workingFinished) {
            return;
        }

        ++_currPackId;
        auto solved = _solvedQ.top();
        _solvedQ.pop();
        _company->solvedPack(solved->getPack());
    }
}

using AFirm = std::shared_ptr<Firm>;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
class COptimizer {
public:
    static bool usingProgtestSolver(void) {
        return true;
    }

    static void checkAlgorithmMin(APolygon p) {
        // dummy implementation if usingProgtestSolver() returns true
    }

    static void checkAlgorithmCnt(APolygon p) {
        // dummy implementation if usingProgtestSolver() returns true
    }

    void start(int threadCount);

    void stop(void);

    void addCompany(ACompany company);

private:
    void working();
    void minSolve(unique_lock<mutex> & workLock);
    void cntSolve(unique_lock<mutex> & workLock);
    size_t _firmCount = 0;
    vector<AFirm> _firms;
    vector<thread> _workingThr;

    Buffer _buffer;
    mutex _mtxWork;
    condition_variable _cvWork;
    atomic<bool> _at_workingFinished = false;
    atomic<bool> _at_receiverFinished = false;

    AProgtestSolver _minSolver = createProgtestMinSolver();
    AProgtestSolver _cntSolver = createProgtestCntSolver();
    vector<AMiniProblem> _minProblemV;
    vector<AMiniProblem> _cntProblemV;
};

void COptimizer::start(int threadCount) {
    for(int i = 0; i < threadCount; ++i) {
        _workingThr.emplace_back(std::move(thread(&COptimizer::working,this)));
    }

    for(auto & firm : _firms) {
        firm->createCommThrs();
    }

}

void COptimizer::stop(void) {
    for(auto & firm : _firms) {
        firm->stopReceiver();
    }
    _at_receiverFinished = true;
    _cvWork.notify_one();
    for(auto & w : _workingThr) {
        w.join();
    }
    _at_workingFinished = true;
    for(auto & firm : _firms) {
        firm->stopDeliver();
    }
}

void COptimizer::addCompany(ACompany company) {
    _firms.emplace_back(make_shared<Firm>(_firmCount++, company, _buffer, _cvWork, _at_workingFinished));
}

void COptimizer::working() {
    while(true) {
        unique_lock workLock(_mtxWork);
        _cvWork.wait(workLock, [&] {return  (!_buffer.empty() || _at_receiverFinished) ;} );
        if(_at_receiverFinished && _buffer.empty() && _minProblemV.empty() && _cntProblemV.empty() ) {
            _cvWork.notify_one();
            return;
        }
        if(_buffer.empty() && _at_receiverFinished) {
            if( !_minProblemV.empty()) {
                minSolve(workLock);
                continue;
            }
            if( !_cntProblemV.empty()) {
                cntSolve(workLock);
                continue;
            }
        }

        while(true) {
            if(_buffer.empty()) {
                break;
            }
            auto miniProblem = _buffer.front();
            _buffer.pop();

            if(miniProblem->getProbType() == ProblemType::MIN) {
                _minProblemV.emplace_back(miniProblem);
                if(! miniProblem->getPolygon().get()) {
                    continue;
                }
                _minSolver->addPolygon(miniProblem->getPolygon());
                if(!_minSolver->hasFreeCapacity()) {
                    minSolve(workLock);
                    break;
                }
            }
            else {
                _cntProblemV.emplace_back(miniProblem);
                if(! miniProblem->getPolygon().get()) {
                    continue;
                }
                _cntSolver->addPolygon(miniProblem->getPolygon());
                if(!_cntSolver->hasFreeCapacity()) {
                    cntSolve(workLock);
                    break;
                }
            }
        }
    }
}

void COptimizer::minSolve(unique_lock<mutex> &workLock) {
    auto oldSolver = _minSolver;
    auto polygons = _minProblemV;
    _minProblemV.clear();
    _minSolver = createProgtestMinSolver();
    workLock.unlock();
    _cvWork.notify_one();
    oldSolver->solve();
    for(auto & x : polygons) {
        _firms[x->getFirmId()]->incProblem(x);
    }
}

void COptimizer::cntSolve(unique_lock<mutex> &workLock) {
    auto oldSolver = _cntSolver;
    auto polygons = _cntProblemV;
    _cntProblemV.clear();
    _cntSolver = createProgtestCntSolver();
    workLock.unlock();
    _cvWork.notify_one();
    oldSolver->solve();
    for(auto & x : polygons) {
        _firms[x->getFirmId()]->incProblem(x);
    }
}
// TODO: COptimizer implementation goes here
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef __PROGTEST__

int main(void) {
    COptimizer optimizer;
    ACompanyTest company = std::make_shared<CCompanyTest>();
    optimizer.addCompany(company);
    optimizer.start(10);
    optimizer.stop();
    if (!company->allProcessed())
        throw std::logic_error("(some) problems were not correctly processsed");
    return 0;
}

#endif /* __PROGTEST__ */
