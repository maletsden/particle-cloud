#ifndef _TIMERCLASS_H_
#define _TIMERCLASS_H_

#include <windows.h>

class TimerClass
{
public:
    bool Initialize();
    void Frame();
    float GetTime() const noexcept;

private:
    INT64 m_frequency;
    float m_ticksPerMs;
    INT64 m_startTime;
    float m_frameTime;
};

#endif