#include "CameraClass.h"

void CameraClass::SetPosition(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    return;
}

void CameraClass::SetRotation(float x, float y, float z)
{
    m_rotationX = x;
    m_rotationY = y;
    m_rotationZ = z;
    return;
}

Vector3 CameraClass::GetPosition() const noexcept
{
    return { m_positionX, m_positionY, m_positionZ };
}

Vector3 CameraClass::GetRotation() const noexcept
{
    return { m_rotationX, m_rotationY, m_rotationZ };
}

void CameraClass::Render()
{
    Vector3 up, position, lookAt;
    float yaw, pitch, roll;
    Matrix rotationMatrix;

    // Setup the vector that points upwards.
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;

    // Setup the position of the camera in the world.
    position.x = m_positionX;
    position.y = m_positionY;
    position.z = m_positionZ;

    // Setup where the camera is looking by default.
    lookAt.x = 0.0f;
    lookAt.y = 0.0f;
    lookAt.z = 1.0f;

    // Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in
    // radians.
    pitch = DirectX::XMConvertToRadians(m_rotationX);
    yaw = DirectX::XMConvertToRadians(m_rotationY);
    roll = DirectX::XMConvertToRadians(m_rotationZ);

    // Create the rotation matrix from the yaw, pitch, and roll values.
    rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Transform the lookAt and up vector by the rotation matrix so the view is
    // correctly rotated at the origin.
    lookAt = DirectX::XMVector3TransformCoord(lookAt, rotationMatrix);
    up = DirectX::XMVector3TransformCoord(up, rotationMatrix);

    // Translate the rotated camera position to the location of the viewer.
    lookAt += position;

    // Finally create the view matrix from the three updated vectors.
    m_viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, up);

    return;
}

const Matrix& CameraClass::GetViewMatrix() const noexcept
{
    return m_viewMatrix;
}
