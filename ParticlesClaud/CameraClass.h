#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

class CameraClass
{
public:
    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);

    Vector3 GetPosition() const noexcept;
    Vector3 GetRotation() const noexcept;

    void Render();
    const Matrix& GetViewMatrix() const noexcept;

private:
    float m_positionX = 0.0f;
    float m_positionY = 0.0f;
    float m_positionZ = 0.0f;

    float m_rotationX = 0.0f;
    float m_rotationY = 0.0f;
    float m_rotationZ = 0.0f;

    Matrix m_viewMatrix;
};

#endif