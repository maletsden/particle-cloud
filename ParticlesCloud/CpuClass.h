#ifndef _CPUCLASS_H_
#define _CPUCLASS_H_

#include <chrono>
#include <pdh.h>

class CpuClass
{
public:
    void Initialize();
    void Shutdown();
    void Frame();
    int GetCpuPercentage() const noexcept;

private:
    bool m_canReadCpu;
    HQUERY m_queryHandle;
    HCOUNTER m_counterHandle;

    std::chrono::system_clock::time_point m_lastSampleTime; 
    long m_cpuUsage;
};

#endif