#include "CpuClass.h"

void CpuClass::Initialize()
{
    PDH_STATUS status;

    // Initialize the flag indicating whether this object can read the system cpu usage or not.
    m_canReadCpu = true;

    // Create a query object to poll cpu usage.
    status = PdhOpenQuery(NULL, 0, &m_queryHandle);
    if (status != ERROR_SUCCESS)
    {
        m_canReadCpu = false;
    }

    // Set query object to poll all cpus in the system.
    status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
    if (status != ERROR_SUCCESS)
    {
        m_canReadCpu = false;
    }

    //m_lastSampleTime = GetTickCount64();
    m_lastSampleTime = std::chrono::system_clock::now();

    m_cpuUsage = 0;

    return;
}

void CpuClass::Shutdown()
{
    if (m_canReadCpu)
    {
        PdhCloseQuery(m_queryHandle);
    }

    return;
}

void CpuClass::Frame()
{
    if (!m_canReadCpu)
    {
        return;
    }

    if ((m_lastSampleTime + std::chrono::seconds(1)) < std::chrono::system_clock::now())
    {
        m_lastSampleTime = std::chrono::system_clock::now();

        PdhCollectQueryData(m_queryHandle);

        PDH_FMT_COUNTERVALUE value;
        PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, nullptr, &value);

        m_cpuUsage = value.longValue;
    }

    return;
}

int CpuClass::GetCpuPercentage() const noexcept
{
    return m_canReadCpu ? static_cast<int>(m_cpuUsage) : 0;
}

