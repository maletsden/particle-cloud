#ifndef _FPSCLASS_H_
#define _FPSCLASS_H_

#include <chrono>

class FpsClass
{
public:
    void Initialize() noexcept;
    void Frame() noexcept;
    int GetFps() const noexcept;

private:
    int m_fps, m_count;
    std::chrono::system_clock::time_point m_startTime;
};

#endif