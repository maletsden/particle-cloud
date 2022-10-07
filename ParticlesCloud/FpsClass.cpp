#include "FpsClass.h"

void FpsClass::Initialize() noexcept
{
    m_fps = 0;
    m_count = 0;
    m_startTime = std::chrono::system_clock::now();
}

void FpsClass::Frame() noexcept
{
    m_count++;

    if (std::chrono::system_clock::now() >= (m_startTime + std::chrono::seconds(1)))
    {
        m_fps = m_count;
        m_count = 0;
        m_startTime = std::chrono::system_clock::now();
    }
}

int FpsClass::GetFps() const noexcept
{
    return m_fps;
}
