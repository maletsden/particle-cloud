#include "TimerClass.h"

bool TimerClass::Initialize()
{
    // Check to see if this system supports high performance timers.
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_frequency));
    if (m_frequency == 0)
    {
        return false;
    }

    // Find out how many times the frequency counter ticks every millisecond.
    m_ticksPerMs = static_cast<float>(m_frequency / 1000);

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_startTime));

    return true;
}

void TimerClass::Frame()
{
    INT64 currentTime;
    float timeDifference;

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));

    timeDifference = static_cast<float>(currentTime - m_startTime);

    m_frameTime = timeDifference / m_ticksPerMs;

    m_startTime = currentTime;

    return;
}

float TimerClass::GetTime() const noexcept
{
    return m_frameTime;
}
