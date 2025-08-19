#ifndef __PROGTEST__

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <stdexcept>
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
const int RAID_RUNNING = 1;
const int BROKEN_DISK_DUMMY = -1;
class RaidData{
public:
    RaidData()
            : _state(RAID_OK), _brokenDisk(-1), _running(RAID_STOPPED)
    {
    }

    bool operator==(const RaidData & right) const {
        return      _state == right._state
                    &&  _brokenDisk == right._brokenDisk;
    }

    void parseToArr(uint8_t * res) const {
        memcpy(res, this, sizeof(RaidData));
    }

    static RaidData parseToData(uint8_t * ptr) {
        RaidData res;
        memcpy(&res, ptr, sizeof(RaidData));
        return res;
    }

    int _state;
    int _brokenDisk;
    int _running;
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
    int getStripe(int secInd) const { return (secInd / (_dev.m_Devices - 1)) + 1; }
    int getParityByStripe(int stripe) const { return (stripe * (_dev.m_Devices - 1)) % _dev.m_Devices; }
    int getDiskBySector(int secNr)  {
        int stripe = getStripe(secNr);
        secNr += stripe - 1;
        secNr %= _dev.m_Devices;

        int parityDisk = getParityByStripe(stripe);
        if(parityDisk <= secNr) {
            secNr++;
        }

        return secNr;
    }

    void dataXOR(uint8_t * restoredData, uint8_t * toXor);

    bool readOK(int secNr, uint8_t * data);
    bool readDEGRADED(int secNr, uint8_t * data);

    bool writeOK(int secNr, uint8_t * data);
    bool writeDEGRADED(int secNr, uint8_t * data);

    RaidData _data;
    TBlkDev _dev;
};

bool CRaidVolume::create(const TBlkDev &dev) {
    if(dev.m_Devices < 3 || dev.m_Devices > 16 || dev.m_Sectors < MIN_DEVICE_SECTORS || dev.m_Sectors > MAX_DEVICE_SECTORS) {
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
    int sectorToWrite = 0;
    _data._running = RAID_RUNNING;
    RaidData datas[MAX_RAID_DEVICES];
    int len = 0;

    for(int i = 0; i < dev.m_Devices; ++i) {
        if(_dev.m_Read(i, sectorToWrite, buffer, 1) != 1) {
        }
        auto temp = RaidData::parseToData(buffer);
        datas[i] = temp;
        ++len;
    }

    RaidData mostRepeat;
    int cntRepeatedMax = 0;

    for(int i = 0; i < len; ++i) {
        int cntRep = 1;
        for(int j = 0; j < len; ++j) {
            if(i != j && datas[i] == datas[j]) {
                ++cntRep;
            }
        }
        if(cntRep > cntRepeatedMax) {
            mostRepeat = datas[i];
            cntRepeatedMax = cntRep;
        }
    }

    _data = mostRepeat;
    _data._running = RAID_RUNNING;
//    for(int i = 0; i < len; ++i) {
//        if(!(mostRepeat == datas[i])) {
//            if(_data._state == RAID_DEGRADED && i != _data._brokenDisk) {
//                _data._state = RAID_FAILED;
//                return RAID_FAILED;
//            }
//            _data._state = RAID_DEGRADED;
//            _data._brokenDisk = i;
//        }
//    }

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

    uint8_t buffer[SECTOR_SIZE];
    uint8_t restoredData[SECTOR_SIZE];
    _data.parseToArr(buffer);

    if(_dev.m_Write(_data._brokenDisk, 0, buffer, 1) != 1) {
        _data._state = RAID_DEGRADED;
        return RAID_DEGRADED;
    }


    for(int stripe = 1; stripe < _dev.m_Sectors; ++stripe) {
        memset(restoredData, 0, SECTOR_SIZE);
        for(int disk = 0; disk < _dev.m_Devices; ++disk) {
            if(disk == _data._brokenDisk) {
                continue;
            }
            if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
                _data._state = RAID_FAILED;
                return RAID_FAILED;
            }
            dataXOR(restoredData, buffer);
        }
        if(_dev.m_Write(_data._brokenDisk, stripe ,restoredData, 1) != 1) {
            _data._state = RAID_DEGRADED;
            return RAID_DEGRADED;
        }
    }
    _data._brokenDisk = BROKEN_DISK_DUMMY;
    _data._state = RAID_OK;
    return RAID_OK;
}

int CRaidVolume::status() const {
    return _data._running == RAID_STOPPED ? RAID_STOPPED : _data._state;
}

int CRaidVolume::size() const {
    return (_dev.m_Devices - 1) * (_dev.m_Sectors - 1);
}

bool CRaidVolume::read(int secNr, void *data, int secCnt) {
    if(_data._state == RAID_FAILED) {
        return false;
    }
    int shift = 0;
    for(; secCnt; --secCnt, ++secNr, ++shift) {
        if(_data._state == RAID_OK) {
            if(readOK(secNr, (uint8_t*)data + (shift * SECTOR_SIZE))) {

            }
        }
        if(_data._state == RAID_DEGRADED) {
            if(readDEGRADED(secNr, (uint8_t*)data + (shift * SECTOR_SIZE))) {

            }
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
    for(; secCnt; --secCnt, ++secNr) {
        if(_data._state == RAID_OK) {
            if(writeOK(secNr, (uint8_t*)data + (shift * SECTOR_SIZE))) {
                ++shift;
            }
        }
        if(_data._state == RAID_DEGRADED) {
            if(writeDEGRADED(secNr, (uint8_t*)data + (shift * SECTOR_SIZE))) {
                ++shift;
            }
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
        _data._brokenDisk = disk;
        return false;
    }
    memcpy(data, buffer, SECTOR_SIZE);

    return true;
}

bool CRaidVolume::readDEGRADED(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    uint8_t buffer[SECTOR_SIZE];

    if(disk == _data._brokenDisk) {
        uint8_t restoredData[SECTOR_SIZE];
        memset(restoredData, 0, SECTOR_SIZE);
        for(int i = 0; i < _dev.m_Devices; ++i) {
            if(i == _data._brokenDisk) {
                continue;
            }
            if( _dev.m_Read(i, stripe, buffer, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            dataXOR(restoredData, buffer);
        }
        memcpy(data, restoredData, SECTOR_SIZE);
    }
    else {
        if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        memcpy(data, buffer, SECTOR_SIZE);

    }

    return true;
}

bool CRaidVolume::writeOK(int secNr, uint8_t *data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    int parityDisk = getParityByStripe(stripe);
    uint8_t parity[SECTOR_SIZE];
    uint8_t oldData[SECTOR_SIZE];
    uint8_t buffer[SECTOR_SIZE];

    if(_dev.m_Read(parityDisk, stripe, parity, 1) != 1) {
        _data._state = RAID_DEGRADED;
        _data._brokenDisk = parityDisk;
        return false;
    }

    if(_dev.m_Read(disk, stripe, oldData, 1) != 1) {
        _data._state = RAID_DEGRADED;
        _data._brokenDisk = disk;
        return false;
    }

    dataXOR(parity, oldData);
    memcpy(buffer, data, SECTOR_SIZE);

    dataXOR(parity, buffer);
    if(_dev.m_Write(disk, stripe, buffer, 1) != 1) {
        _data._state = RAID_DEGRADED;
        _data._brokenDisk = disk;
        return false;
    }
    if(_dev.m_Write(parityDisk, stripe, parity, 1) != 1) {
        _data._state = RAID_DEGRADED;
        _data._brokenDisk = parityDisk;
        return false;
    }

    return true;
}

bool CRaidVolume::writeDEGRADED(int secNr, uint8_t * data) {
    int stripe = getStripe(secNr);
    int disk = getDiskBySector(secNr);
    int parityDisk = getParityByStripe(stripe);
    uint8_t buffer[SECTOR_SIZE];
    uint8_t newData[SECTOR_SIZE];

    memcpy(newData, data, SECTOR_SIZE);

    if(parityDisk == _data._brokenDisk) {
        if(_dev.m_Write(disk, stripe, newData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
    }
    else if(disk == _data._brokenDisk) {
        for(int i = 0; i < _dev.m_Devices; ++i) {
            if(i == disk || i == parityDisk) {
                continue;
            }
            if(_dev.m_Read(i, stripe, buffer, 1) != 1) {
                _data._state = RAID_FAILED;
                return false;
            }
            dataXOR(newData, buffer);
        }
        if(_dev.m_Write(parityDisk, stripe, newData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
    }
    else {
        uint8_t parity[SECTOR_SIZE];
        if(_dev.m_Read(parityDisk, stripe, parity, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }

        if(_dev.m_Read(disk, stripe, buffer, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }

        dataXOR(parity, buffer);
        dataXOR(parity, newData);

        if(_dev.m_Write(disk, stripe, newData, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
        if(_dev.m_Write(parityDisk, stripe, parity, 1) != 1) {
            _data._state = RAID_FAILED;
            return false;
        }
    }

    return true;
}

void CRaidVolume::dataXOR(uint8_t *restoredData, uint8_t *toXor) {
    for(int i = 0; i < SECTOR_SIZE; ++i) {
        restoredData[i] ^= toXor[i];
    }
}