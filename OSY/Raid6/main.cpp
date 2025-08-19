#ifndef __PROGTEST__

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>

using namespace std;

constexpr int SECTOR_SIZE = 512;
constexpr int MAX_RAID_DEVICES = 16;
constexpr int MAX_DEVICE_SECTORS = 1024 * 1024 * 2;
constexpr int MIN_DEVICE_SECTORS = 1 * 1024 * 2;

constexpr int RAID_STOPPED = 0;
constexpr int RAID_OK = 1;
constexpr int RAID_DEGRADED = 2;
constexpr int RAID_FAILED = 3;

struct TBlkDev {
    int m_Devices;
    int m_Sectors;

    int (*m_Read )(int, int, void *, int);

    int (*m_Write )(int, int, const void *, int);
};

#endif /* __PROGTEST__ */
#define H_SIZE 256
class RSHelper{
public:
    RSHelper();
    uint8_t getInfo(int ind);
    uint8_t add(uint8_t base, uint8_t toXor);
    uint8_t mult(uint8_t left, uint8_t right);
    uint8_t division(uint8_t left, uint8_t right);

    uint8_t _table1[H_SIZE];
    uint8_t _table2[H_SIZE];
};

RSHelper::RSHelper() {
    const int VAR_FOR_XOR = 285;
    memset(_table1, 0, H_SIZE);
    memset(_table2, 0, H_SIZE);
    for(int i = 0, j = 1; i < H_SIZE; ++i) {
        _table1[j] = i & (H_SIZE - 1);
        _table2[i] = j & (H_SIZE - 1);
        j <<= 1;
        if(j & H_SIZE) {
            j ^= VAR_FOR_XOR;
        }
    }
    _table1[1] = 0;
}

uint8_t RSHelper::getInfo(int ind) {
    return _table2[ind];
}

uint8_t RSHelper::add(uint8_t base, uint8_t toXor) {
    return base ^ toXor;
}

uint8_t RSHelper::mult(uint8_t left, uint8_t right) {
    if(!left || !right) {
        return 0;
    }
    int temp = _table1[left] + _table1[right];
    temp %= H_SIZE - 1;
    return _table2[temp];
}

uint8_t RSHelper::division(uint8_t left, uint8_t right) {
    int temp = _table1[left] - _table1[right];
    if(_table1[left] <= _table1[right]) {
        temp += H_SIZE - 1;
    }
    return _table2[temp];
}



const int RAID_RUNNING = 1;
const int BROKEN_DISK_DUMMY = -1;
class RaidData{
public:
    RaidData()
            : _state(RAID_OK), _running(RAID_STOPPED), _brokenFirst(BROKEN_DISK_DUMMY), _brokenSecond(BROKEN_DISK_DUMMY)
    {
    }

    bool operator==(const RaidData & right) const {
        return          _state == right._state
                    &&  _brokenFirst == right._brokenFirst
                    &&  _brokenSecond == right._brokenSecond;
    }
    bool operator!=(const RaidData & right) const {
        return !(*this == right);
    }

    void parseToArr(uint8_t * res) const {
        memcpy(res, this, sizeof(RaidData));
    }

    static RaidData parseToData(uint8_t * ptr) {
        RaidData res;
        memcpy(&res, ptr, sizeof(RaidData));
        return res;
    }
    bool isOneBroken() const {return (_brokenFirst != BROKEN_DISK_DUMMY && _brokenSecond == BROKEN_DISK_DUMMY) || (_brokenFirst == BROKEN_DISK_DUMMY && _brokenSecond != BROKEN_DISK_DUMMY) ; }
    bool isTwoBroken() const { return _brokenFirst != BROKEN_DISK_DUMMY && _brokenSecond != BROKEN_DISK_DUMMY; }

    bool isDiskBroken(int disk) const { return _brokenFirst == disk || _brokenSecond == disk; }

    int _state;
    int _running;
    int _brokenFirst;
    int _brokenSecond;
};


class CRaidVolume {
public:
    static bool create(const TBlkDev &dev);
    int start(const TBlkDev &dev);
    int stop();
    int resync();
    int status() const;
    int size() const;
    bool read(int secNr, void *data, int secCnt);
    bool write(int secNr, const void *data, int secCnt);

protected:
    int getStripe(int secInd) const { return (secInd / (_dev.m_Devices - 2)) + 1; }
    /* первый диск XOR второй SOLOMON GRANDI  */
    pair<int,int> getParityByStripe(int stripe) const {
        int first = (stripe * (_dev.m_Devices - 1)) % _dev.m_Devices;
        int second = (first - 1);
        if(second < 0) {
            second += _dev.m_Devices;
        }
        return make_pair(first,second);
    }
    int getDiskBySector(int secNr)  {
        int stripe = getStripe(secNr);
        secNr += (stripe - 1) * 2;
        secNr %= _dev.m_Devices;
        auto pairs = getParityByStripe(stripe);
        for(int i = 0; i < _dev.m_Devices && i <= secNr; ++i) {
            if(i == pairs.first || i == pairs.second) {
                ++secNr;
            }
        }
        return secNr;
    }


    bool repairOneXOR(int stripe, const pair<int, int> & pairs);
    bool repairOneRS(int stripe, const pair<int, int> & pairs);
    bool repairOneDATA(int stripe, const pair<int, int> & pairs);

    bool repairTwoXOR_RS(int stripe, const pair<int, int> & pairs);
    bool repairTwoDATA_XOR(int stripe, const pair<int, int> & pairs);
    bool repairTwoDATA_RS(int stripe, const pair<int, int> & pairs);
    bool repairTwoDATA_DATA(int stripe, const pair<int, int> & pairs);

    bool readOK(int secNr, uint8_t * data);
    bool readOneBroken(int secNr, uint8_t * data);
    bool readTwoBroken(int secNr, uint8_t * data);
    bool readBrokenDataRS(int stripe, int disk, const pair<int,int> & pairs, uint8_t * data);
    bool readBrokenDataXOR(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool readBrokenDataData(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);

    bool writeOK(int secNr, uint8_t * data);
    bool writeOneBroken(int secNr, uint8_t * data);
    bool writeTwoBroken(int secNr, uint8_t * data);

    bool writeBrokenOneRS(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool writeBrokenOneXOR(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool writeBrokenOneDataOnWriting(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool writeBrokenOneDataOn_NOT_Writing(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);


    bool writeBrokenTWODataRS(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool writeBrokenTWODataXOR(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);
    bool writeBrokenTWODataData(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data);

    void twoDataXOR(uint8_t * restoredData, uint8_t * toXor);
    void multInfoData(int disk, uint8_t * bufferData);

    RSHelper helper;
    RaidData _data;
    TBlkDev _dev;
};

bool CRaidVolume::create(const TBlkDev &dev) {
    if(dev.m_Devices < 4 || dev.m_Devices > MAX_RAID_DEVICES || dev.m_Sectors < MIN_DEVICE_SECTORS || dev.m_Sectors > MAX_DEVICE_SECTORS) {
        return false;
    }
    RaidData initData;
    uint8_t buffer[SECTOR_SIZE];
    initData.parseToArr(buffer);
    for(int i = 0; i < dev.m_Devices; ++i) {
        dev.m_Write(i, 0, buffer, 1);
    }
    return true;
}

int CRaidVolume::start(const TBlkDev &dev) {
    _dev = dev;
    uint8_t buffer[SECTOR_SIZE];
    _data = RaidData();
    _data._running = RAID_RUNNING;
    int sectorToWrite = 0;
    RaidData datas[MAX_RAID_DEVICES];
    int len = 0;

    for(int i = 0; i < dev.m_Devices; ++i) {
        if(_dev.m_Read(i, sectorToWrite, buffer, 1) != 1) {
            if(_data.isTwoBroken()) {
                _data._state = RAID_FAILED;
                return _data._state;
            }
            else if(_data.isOneBroken() && _data._state == RAID_DEGRADED) {
                _data._brokenSecond = i;
            }
            else {
                _data._brokenFirst = i;
                _data._state = RAID_DEGRADED;
            }
        }
        auto temp = RaidData::parseToData(buffer);
        datas[i] = temp;
        ++len;
    }

    RaidData mostRepeat;
    int cntRepeatedMax = 0;

    for(int i = 0; i < len; ++i) {
        if(_data.isDiskBroken(i)) {
            continue;
        }
        int cntRep = 1;
        for(int j = 0; j < len; ++j) {
            if(_data.isDiskBroken(j)) {
                continue;
            }
            if(i != j && datas[i] == datas[j]) {
                ++cntRep;
            }
        }
        if(cntRep > cntRepeatedMax) {
            mostRepeat = datas[i];
            cntRepeatedMax = cntRep;
        }
    }

//    if(mostRepeat._state == RAID_FAILED) {
//        _data._state = RAID_FAILED;
//        return _data._state;
//    }
//    if(_data.isTwoBroken() && mostRepeat.isTwoBroken()) {
//        if(!_data.isDiskBroken(mostRepeat._brokenFirst) || !_data.isDiskBroken(mostRepeat._brokenSecond)) {
//            _data._state = RAID_FAILED;
//            return _data._state;
//        }
//    }
//    else if(_data.isTwoBroken() && mostRepeat.isOneBroken()) {
//        if(!_data.isDiskBroken(mostRepeat._brokenFirst)) {
//            _data._state = RAID_FAILED;
//            return _data._state;
//        }
//    }
//    else if(_data.isOneBroken() && mostRepeat.isTwoBroken()) {
//        if(!mostRepeat.isDiskBroken(_data._brokenFirst)) {
//            _data._state = RAID_FAILED;
//            return _data._state;
//        }
//    }
//    else if(_data.isOneBroken() && mostRepeat.isOneBroken()) {
//        if(_data._brokenFirst != mostRepeat._brokenFirst) {
//            _data._brokenSecond = mostRepeat._brokenSecond;
//            return _data._state;
//        }
//    }
//
    _data = mostRepeat;
    _data._running = RAID_RUNNING;
    return _data._state;
}

int CRaidVolume::stop() {
    _data._running = RAID_STOPPED;
    uint8_t buffer[SECTOR_SIZE];
    _data.parseToArr(buffer);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        _dev.m_Write(i, 0, buffer, 1);
    }
    return RAID_STOPPED;
}

int CRaidVolume::resync() {
    if(_data._state == RAID_FAILED) {
        return RAID_FAILED;
    }

    if(_data.isOneBroken()) {
        for(int stripe = 1; stripe < _dev.m_Sectors; ++stripe) {
            auto pairs = getParityByStripe(stripe);
            if(_data._brokenFirst == pairs.first) {
                if(!repairOneXOR(stripe, pairs)) {
                    return _data._state;
                }
            }
            else if(_data._brokenFirst == pairs.second) {
                if(!repairOneRS(stripe, pairs)) {
                    return _data._state;
                }
            }
            else {
                if(!repairOneDATA(stripe, pairs)) {
                    return _data._state;
                }
            }
        }
    }
    else {
        for(int stripe = 1; stripe < _dev.m_Sectors; ++stripe) {
            auto pairs = getParityByStripe(stripe);
            if(_data.isDiskBroken(pairs.first) && _data.isDiskBroken(pairs.second)) {
                if(!repairTwoXOR_RS(stripe, pairs)) {
                    return _data._state;
                }
            }
            if(_data.isDiskBroken(pairs.first)) {
                if(!repairTwoDATA_XOR(stripe, pairs)) {
                    return _data._state;
                }
            }
            else if(_data.isDiskBroken(pairs.second)) {
                if(!repairTwoDATA_RS(stripe, pairs)) {
                    return _data._state;
                }
            }
            else {
                if(!repairTwoDATA_DATA(stripe, pairs)) {
                    return _data._state;
                }
            }
        }
    }

    _data._brokenFirst = BROKEN_DISK_DUMMY;
    _data._brokenSecond = BROKEN_DISK_DUMMY;
    _data._state = RAID_OK;
    return RAID_OK;
}

int CRaidVolume::status() const {
    return _data._running == RAID_STOPPED ? RAID_STOPPED : _data._state;
}

int CRaidVolume::size() const {
    return (_dev.m_Devices - 2) * (_dev.m_Sectors - 1);
}

bool CRaidVolume::read(int secNr, void *data, int secCnt) {
    if(_data._state == RAID_FAILED) {
        return false;
    }
    int shift = 0;
    for(; secCnt; --secCnt, ++secNr, ++shift) {
        if(_data._state == RAID_OK) {
            readOK(secNr, (uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data.isOneBroken() && _data._state == RAID_DEGRADED) {
            readOneBroken(secNr, (uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data.isTwoBroken() && _data._state == RAID_DEGRADED) {
            readTwoBroken(secNr,(uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data._state == RAID_FAILED) {
            return false;
        }
    }
    return true;
}
bool CRaidVolume::write(int secNr, const void *data, int secCnt) {
    if(_data._state == RAID_FAILED) {
        return false;
    }
    int shift = 0;
    for(; secCnt; --secCnt, ++secNr, ++shift) {
        if(_data._state == RAID_OK) {
            writeOK(secNr, (uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data.isOneBroken() && _data._state == RAID_DEGRADED) {
            writeOneBroken(secNr, (uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data.isTwoBroken() && _data._state == RAID_DEGRADED) {
            writeTwoBroken(secNr,(uint8_t*)data + (shift * SECTOR_SIZE));
        }
        if(_data._state == RAID_FAILED) {
            return false;
        }
    }
    return true;
}
bool CRaidVolume::readOK(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    uint8_t buffer[SECTOR_SIZE];

    if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
        _data._state = RAID_DEGRADED;
        _data._brokenFirst = disk;
        return false;
    }
    memcpy(data, buffer, SECTOR_SIZE);
    return true;
}
bool CRaidVolume::readOneBroken(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    auto pairs = getParityByStripe(stripe);
//    int solParity = pairs.second;
    uint8_t buffer[SECTOR_SIZE];

    if(_data.isDiskBroken(disk)) {
        uint8_t restoredData[SECTOR_SIZE];
        memset(restoredData, 0, SECTOR_SIZE);
        for(int i = 0; i < _dev.m_Devices; ++i) {
            if(i == _data._brokenFirst || i == pairs.second) {
                continue;
            }
            if( _dev.m_Read(i, stripe, buffer, 1) != 1) {
                _data._brokenSecond = i;
                return false;
            }
            twoDataXOR(restoredData, buffer);
        }
        memcpy(data, restoredData, SECTOR_SIZE);
    }
    else {
        if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
            _data._brokenSecond = disk;
            return false;
        }
        memcpy(data, buffer, SECTOR_SIZE);
    }

    return true;
}
bool CRaidVolume::readTwoBroken(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    auto pairs = getParityByStripe(stripe);

    if(!_data.isDiskBroken(disk)) {
        uint8_t buffer[SECTOR_SIZE];
        if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        memcpy(data, buffer, SECTOR_SIZE);
        return true;
    }
    //sloman data disk i paritaXor
    else if(_data.isDiskBroken(pairs.first)) {
        return readBrokenDataXOR(stripe, disk, pairs, data);
    }
    //sloman solomon i data disk
    else if(_data.isDiskBroken(pairs.second)) {
        return readBrokenDataRS(stripe, disk, pairs, data);
    }

    return readBrokenDataData(stripe, disk, pairs, data);
}
bool CRaidVolume::readBrokenDataXOR(int stripe, int disk, const pair<int,int> & pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t tempPart[SECTOR_SIZE];
    memset(tempPart, 0, SECTOR_SIZE);

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        else if(i == disk) {
            if(_dev.m_Read(pairs.second, stripe, bufferData, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            twoDataXOR(tempPart, bufferData);
        }
        else {
            if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }

            multInfoData(i, bufferData);
            twoDataXOR(tempPart, bufferData);
        }
    }

    uint8_t restoredData[SECTOR_SIZE];
    uint8_t div = helper.division(1, helper.getInfo(disk));

    for(int i = 0; i < SECTOR_SIZE; ++i) {
        restoredData[i] = helper.mult(div, tempPart[i]);
    }

    memcpy(data, restoredData, SECTOR_SIZE);
    return true;
}
bool CRaidVolume::readBrokenDataRS(int stripe, int disk, const pair<int,int> & pairs, uint8_t * data) {
    uint8_t buffer[SECTOR_SIZE];
    uint8_t restoredData[SECTOR_SIZE];
    memset(restoredData, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(_data.isDiskBroken(i)) {
            continue;
        }
        if( _dev.m_Read(i, stripe, buffer, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(restoredData, buffer);
    }
    memcpy(data, restoredData, SECTOR_SIZE);
    return true;
}
bool CRaidVolume::readBrokenDataData(int stripe, int disk, const pair<int,int> & pairs, uint8_t * data) {
    uint8_t tempXOR[SECTOR_SIZE];
    uint8_t tempRS[SECTOR_SIZE];
    memset(tempXOR, 0, SECTOR_SIZE);
    memset(tempRS, 0, SECTOR_SIZE);

    uint8_t bufferData[SECTOR_SIZE];
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(_data.isDiskBroken(i) || i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(tempXOR, bufferData);

        multInfoData(i, bufferData);
        twoDataXOR(tempRS, bufferData);
    }

    if(_dev.m_Read(pairs.first, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempXOR, bufferData);

    if(_dev.m_Read(pairs.second, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempRS, bufferData);

    int minDisk = _data._brokenFirst < _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    int maxDisk = _data._brokenFirst > _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    uint8_t xzAdd = helper.add(helper.getInfo(minDisk), helper.getInfo(maxDisk));
    uint8_t xz = helper.division(1, xzAdd);

    uint8_t xorREG[SECTOR_SIZE];
    memcpy(xorREG, tempXOR, SECTOR_SIZE);
    multInfoData(maxDisk, xorREG);
    twoDataXOR(xorREG, tempRS);

    uint8_t minData[SECTOR_SIZE];
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        minData[i] = helper.mult(xorREG[i], xz);
    }

    if(minDisk != disk) {
        twoDataXOR(minData, tempXOR);
    }

    memcpy(data, minData, SECTOR_SIZE);
    return true;

}
bool CRaidVolume::writeOK(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    auto pairs = getParityByStripe(stripe);
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == disk) {
            memcpy(bufferData, data, SECTOR_SIZE);
        }
        else if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenFirst = i;
            _data._state = RAID_DEGRADED;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }


    uint8_t parityXOR[SECTOR_SIZE];

    // zhana XOR PARITY
    if(_dev.m_Read(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenFirst = pairs.first;
        _data._state = RAID_DEGRADED;
        return false;
    }
    if(_dev.m_Read(disk, stripe, bufferData, 1) != 1) {
        _data._brokenFirst = disk;
        _data._state = RAID_DEGRADED;
        return false;
    }

    twoDataXOR(parityXOR, bufferData);
    memcpy(bufferData, data, SECTOR_SIZE);
    twoDataXOR(parityXOR, bufferData);
    int temp;
    memcpy(&temp, parityXOR, 4);
    if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
        _data._brokenFirst = disk;
        _data._state = RAID_DEGRADED;
        return false;
    }

    if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenFirst = pairs.first;
        _data._state = RAID_DEGRADED;
        return false;
    }

    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        _data._brokenFirst = pairs.second;
        _data._state = RAID_DEGRADED;
        return false;
    }

    return true;
}
bool CRaidVolume::writeOneBroken(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    auto pairs = getParityByStripe(stripe);

    //RS parita
    if(_data.isDiskBroken(pairs.second)) {
        return writeBrokenOneRS(stripe, disk, pairs, data);
    }
    //XOR PARITA
    if(_data.isDiskBroken(pairs.first)) {
        return writeBrokenOneXOR(stripe, disk, pairs, data);
    }
    // ZHAZUHA DISK DEAD
    if(_data.isDiskBroken(disk)) {
        return writeBrokenOneDataOnWriting(stripe, disk, pairs, data);
    }
    // umer baska disk
    return writeBrokenOneDataOn_NOT_Writing(stripe, disk, pairs, data);
}
bool CRaidVolume::writeBrokenOneRS(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    // zhana XOR PARITY
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    if(_dev.m_Read(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenSecond = pairs.first;
        return false;
    }
    if(_dev.m_Read(disk, stripe, bufferData, 1) != 1) {
        _data._brokenSecond = disk;
        return false;
    }

    twoDataXOR(parityXOR, bufferData);
    memcpy(bufferData, data, SECTOR_SIZE);
    twoDataXOR(parityXOR, bufferData);

    if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
        _data._brokenSecond = disk;
        return false;
    }

    if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenSecond = pairs.first;
        return false;
    }
    return true;
}
bool CRaidVolume::writeBrokenOneXOR(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == disk) {
            memcpy(bufferData, data, SECTOR_SIZE);
//            multInfoData(i, bufferData);
//            twoDataXOR(parityRS, bufferData);
//            continue;
        }
        else if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }
    memcpy(bufferData, data, SECTOR_SIZE);
    if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
        _data._brokenSecond = disk;
        return false;
    }

    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        _data._brokenSecond = pairs.second;
        return false;
    }
    return true;
}
bool CRaidVolume::writeBrokenOneDataOnWriting(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityXOR, 0 , SECTOR_SIZE);
    memset(parityRS, 0, SECTOR_SIZE);

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == disk) {
            memcpy(bufferData, data, SECTOR_SIZE);
            twoDataXOR(parityXOR, bufferData);
            multInfoData(i, bufferData);
            twoDataXOR(parityRS, bufferData);
            continue;
        }

        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }

        twoDataXOR(parityXOR, bufferData);
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }

    if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenSecond = pairs.first;
        return false;
    }

    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        _data._brokenSecond = pairs.second;
        return false;
    }

    return true;
}
bool CRaidVolume::writeBrokenOneDataOn_NOT_Writing(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t missDiskPar[SECTOR_SIZE];
    memset(missDiskPar, 0, SECTOR_SIZE);

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.second || _data.isDiskBroken(i)) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        twoDataXOR(missDiskPar, bufferData);
    }

    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == disk) {
            memcpy(bufferData, data, SECTOR_SIZE);
            multInfoData(i, bufferData);
            twoDataXOR(parityRS, bufferData);
            continue;
        }

        if(_data.isDiskBroken(i)) {
            memcpy(bufferData, missDiskPar, SECTOR_SIZE);
            multInfoData(i, bufferData);
            twoDataXOR(parityRS, bufferData);
            continue;
        }

        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }

    uint8_t parityXOR[SECTOR_SIZE];
    if(_dev.m_Read(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenSecond = pairs.first;
        return false;
    }
    if(_dev.m_Read(disk, stripe, bufferData, 1) != 1) {
        _data._brokenSecond = disk;
        return false;
    }

    twoDataXOR(parityXOR, bufferData);
    memcpy(bufferData, data, SECTOR_SIZE);
    twoDataXOR(parityXOR, bufferData);

    if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
        _data._brokenSecond = disk;
        return false;
    }

    if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._brokenSecond = pairs.first;
        return false;
    }

    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        _data._brokenSecond = pairs.second;
        return false;
    }

    return true;
}
bool CRaidVolume::writeTwoBroken(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    auto pairs = getParityByStripe(stripe);

    if(_data.isDiskBroken(pairs.first) && _data.isDiskBroken(pairs.second)) {
        uint8_t buffer[SECTOR_SIZE];
        memcpy(buffer, data, SECTOR_SIZE);
        if(_dev.m_Write(disk, stripe, buffer, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        return true;
    }
    if(_data.isDiskBroken(pairs.first)) {
        return writeBrokenTWODataXOR(stripe, disk, pairs, data);
    }

    if(_data.isDiskBroken(pairs.second)) {
        return writeBrokenTWODataRS(stripe, disk, pairs, data);
    }

    return writeBrokenTWODataData(stripe, disk, pairs, data);

    return false;

}
bool CRaidVolume::writeBrokenTWODataRS(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    memset(parityXOR, 0, SECTOR_SIZE);
    if(_data.isDiskBroken(disk)) {
        for(int i = 0; i < _dev.m_Devices; ++i) {
            if(i == pairs.first || i == pairs.second || i == disk) {
                continue;
            }
            if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            twoDataXOR(parityXOR, bufferData);
        }
        memcpy(bufferData, data, SECTOR_SIZE);
        twoDataXOR(parityXOR, bufferData);
        if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        return true;
    }
    if(_dev.m_Read(disk, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    if(_dev.m_Read(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    twoDataXOR(parityXOR, bufferData);
    memcpy(bufferData, data, SECTOR_SIZE);
    twoDataXOR(parityXOR, bufferData);

    if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    if(_dev.m_Write(pairs.first, stripe, parityXOR, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }



    return true;
}
bool CRaidVolume::writeBrokenTWODataXOR(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);

    if(_data.isDiskBroken(disk)) {
        for(int i = 0; i < _dev.m_Devices; ++i) {
            if(i == pairs.first || i == pairs.second) {
                continue;
            }
            if(i == disk) {
                memcpy(bufferData, data, SECTOR_SIZE);
                multInfoData(i, bufferData);
                twoDataXOR(parityRS, bufferData);
                continue;
            }
            if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            multInfoData(i, bufferData);
            twoDataXOR(parityRS, bufferData);
        }
        if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        return true;
    }

    if(_dev.m_Read(pairs.second, stripe, parityRS, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    uint8_t tempPart[SECTOR_SIZE];
    memset(tempPart, 0, SECTOR_SIZE);

    int brokenDisk = _data._brokenFirst != pairs.first ? _data._brokenFirst : _data._brokenSecond;
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == brokenDisk) {
            twoDataXOR(tempPart, parityRS);
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(tempPart, bufferData);
    }

    uint8_t restoredData[SECTOR_SIZE];
    uint8_t div = helper.division(1, helper.getInfo(brokenDisk));
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        restoredData[i] = helper.mult(div, tempPart[i]);
    }

    memcpy(bufferData, data ,SECTOR_SIZE);
    if(_dev.m_Write(disk, stripe, bufferData, 1 ) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    memset(parityRS, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == brokenDisk) {
            multInfoData(i, restoredData);
            twoDataXOR(parityRS, restoredData);
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }

    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    return true;
}
bool CRaidVolume::writeBrokenTWODataData(int stripe, int disk, const pair<int, int> &pairs, uint8_t *data) {
    uint8_t tempXOR[SECTOR_SIZE];
    uint8_t tempRS[SECTOR_SIZE];
    memset(tempXOR, 0, SECTOR_SIZE);
    memset(tempRS, 0, SECTOR_SIZE);

    uint8_t bufferData[SECTOR_SIZE];
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(_data.isDiskBroken(i) || i == pairs.first || i == pairs.second) {
            continue;
        }
        else if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(tempXOR, bufferData);

        multInfoData(i, bufferData);
        twoDataXOR(tempRS, bufferData);
    }

    if(_dev.m_Read(pairs.first, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempXOR, bufferData);

    if(_dev.m_Read(pairs.second, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempRS, bufferData);

    int minDisk = _data._brokenFirst < _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    int maxDisk = _data._brokenFirst > _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    uint8_t xzAdd = helper.add(helper.getInfo(minDisk), helper.getInfo(maxDisk));
    uint8_t xz = helper.division(1, xzAdd);

    uint8_t xorREG[SECTOR_SIZE];
    memcpy(xorREG, tempXOR, SECTOR_SIZE);
    multInfoData(maxDisk, xorREG);
    twoDataXOR(xorREG, tempRS);

    uint8_t minData[SECTOR_SIZE];
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        minData[i] = helper.mult(xorREG[i], xz);
    }
    uint8_t maxData[SECTOR_SIZE];
    memcpy(maxData, minData, SECTOR_SIZE);
    twoDataXOR(maxData, tempXOR);

//    if(_dev.m_Read(pairs.first, stripe, tempXOR, 1) != 1) {
//        _data._state = RAID_FAILED;
//        return false;
//    }

    memset(tempRS, 0, SECTOR_SIZE);
    memset(tempXOR, 0, SECTOR_SIZE);
    if(disk == minDisk) {
        memcpy(minData, data, SECTOR_SIZE);
    }
    else if(disk == maxDisk) {
        memcpy(maxData, data, SECTOR_SIZE);
    }
    else {
        memcpy(bufferData, data,SECTOR_SIZE);
        if(_dev.m_Write(disk, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
    }

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == minDisk) {
            twoDataXOR(tempXOR, minData);
            multInfoData(i, minData);
            twoDataXOR(tempRS, minData);
        }
        else if(i == maxDisk) {
            twoDataXOR(tempXOR, maxData);
            multInfoData(i, maxData);
            twoDataXOR(tempRS, maxData);
        }
        else {
            if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            twoDataXOR(tempXOR, bufferData);
            multInfoData(i, bufferData);
            twoDataXOR(tempRS, bufferData);
        }
    }

    if(_dev.m_Write(pairs.first, stripe, tempXOR, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    if(_dev.m_Write(pairs.second, stripe, tempRS, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    return true;
}

bool CRaidVolume::repairOneXOR(int stripe, const pair<int, int> & pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    memset(parityXOR, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        twoDataXOR(parityXOR, bufferData);
    }

    if(_dev.m_Write(_data._brokenFirst, stripe, parityXOR, 1) != 1) {
        return false;
    }
    return true;
}
bool CRaidVolume::repairOneRS(int stripe, const pair<int, int> &pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }
    if(_dev.m_Write(_data._brokenFirst, stripe, parityRS, 1) != 1) {
        return false;
    }
    return true;
}
bool CRaidVolume::repairOneDATA(int stripe, const pair<int, int> &pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    memset(parityXOR, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == _data._brokenFirst || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._brokenSecond = i;
            return false;
        }
        twoDataXOR(parityXOR, bufferData);
    }
    if(_dev.m_Write(_data._brokenFirst, stripe, parityXOR, 1) != 1) {
        return false;
    }
    return true;
}

bool CRaidVolume::repairTwoDATA_XOR(int stripe, const pair<int, int> & pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);

    if(_dev.m_Read(pairs.second, stripe, parityRS, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }

    uint8_t tempPart[SECTOR_SIZE];
    memset(tempPart, 0, SECTOR_SIZE);

    int brokenDiskData = _data._brokenFirst != pairs.first ? _data._brokenFirst : _data._brokenSecond;
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(i == brokenDiskData) {
            twoDataXOR(tempPart, parityRS);
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(tempPart, bufferData);
    }

    uint8_t restoredData[SECTOR_SIZE];
    uint8_t div = helper.division(1, helper.getInfo(brokenDiskData));
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        restoredData[i] = helper.mult(div, tempPart[i]);
    }

    if(_dev.m_Write(brokenDiskData, stripe, restoredData, 1 ) != 1) {
        return false;
    }

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second || i == brokenDiskData) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(restoredData, bufferData);
    }
    if(_dev.m_Write(pairs.first, stripe, restoredData, 1) != 1) {
        return false;
    }

    return true;
}
bool CRaidVolume::repairTwoDATA_RS(int stripe, const pair<int, int> & pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    memset(parityXOR, 0, SECTOR_SIZE);
    int brokenDiskData = _data._brokenFirst != pairs.second ? _data._brokenFirst : _data._brokenSecond;

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == brokenDiskData || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(parityXOR, bufferData);
    }

    if(_dev.m_Write(brokenDiskData, stripe, parityXOR, 1) != 1) {
        return false;
    }

    uint8_t parityRS[SECTOR_SIZE];
    memset(parityRS, 0, SECTOR_SIZE);
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }
    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        return false;
    }

    return true;
}
bool CRaidVolume::repairTwoDATA_DATA(int stripe, const pair<int, int> & pairs) {
    uint8_t tempXOR[SECTOR_SIZE];
    uint8_t tempRS[SECTOR_SIZE];
    memset(tempXOR, 0, SECTOR_SIZE);
    memset(tempRS, 0, SECTOR_SIZE);

    uint8_t bufferData[SECTOR_SIZE];
    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(_data.isDiskBroken(i) || i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(tempXOR, bufferData);

        multInfoData(i, bufferData);
        twoDataXOR(tempRS, bufferData);
    }

    if(_dev.m_Read(pairs.first, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempXOR, bufferData);

    if(_dev.m_Read(pairs.second, stripe, bufferData, 1) != 1) {
        _data._state = RAID_FAILED;
        return false;
    }
    twoDataXOR(tempRS, bufferData);

    int minDisk = _data._brokenFirst < _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    int maxDisk = _data._brokenFirst > _data._brokenSecond ? _data._brokenFirst : _data._brokenSecond;
    uint8_t xzAdd = helper.add(helper.getInfo(minDisk), helper.getInfo(maxDisk));
    uint8_t xz = helper.division(1, xzAdd);

    uint8_t xorREG[SECTOR_SIZE];
    memcpy(xorREG, tempXOR, SECTOR_SIZE);
    multInfoData(maxDisk, xorREG);
    twoDataXOR(xorREG, tempRS);

    uint8_t minData[SECTOR_SIZE];
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        minData[i] = helper.mult(xorREG[i], xz);
    }
    uint8_t maxData[SECTOR_SIZE];
    memcpy(maxData, minData, SECTOR_SIZE);
    twoDataXOR(maxData, tempXOR);

    if(_dev.m_Write(minDisk, stripe, minData, 1) != 1) {
        return false;
    }

    if(_dev.m_Write(maxDisk, stripe, maxData, 1) != 1) {
        return false;
    }

    return true;
}

void CRaidVolume::twoDataXOR(uint8_t *restoredData, uint8_t *toXor) {
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        restoredData[i] ^= toXor[i];
    }
}
void CRaidVolume::multInfoData(int disk, uint8_t * bufferData) {
    for(int jS = 0; jS < SECTOR_SIZE; ++jS) {
        bufferData[jS] = helper.mult(helper.getInfo(disk), bufferData[jS]);
    }
}

bool CRaidVolume::repairTwoXOR_RS(int stripe, const pair<int, int> &pairs) {
    uint8_t bufferData[SECTOR_SIZE];
    uint8_t parityXOR[SECTOR_SIZE];
    uint8_t parityRS[SECTOR_SIZE];
    memset(parityXOR, 0, SECTOR_SIZE);
    memset(parityRS, 0, SECTOR_SIZE);

    for(int i = 0; i < _dev.m_Devices; ++i) {
        if(i == pairs.first || i == pairs.second) {
            continue;
        }
        if(_dev.m_Read(i, stripe, bufferData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        twoDataXOR(parityXOR, bufferData);
        multInfoData(i, bufferData);
        twoDataXOR(parityRS, bufferData);
    }

    if(_dev.m_Read(pairs.first, stripe, parityXOR, 1) != 1) {
        return false;
    }
    if(_dev.m_Write(pairs.second, stripe, parityRS, 1) != 1) {
        return false;
    }
    return true;
}

#ifndef __PROGTEST__

#include "tests.inc"
#endif /* __PROGTEST__ */
