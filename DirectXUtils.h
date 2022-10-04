#pragma once

#include <windows.h>

#include <d3d11.h>

namespace DirectXUtils
{
    //--------------------------------------------------------------------------------------
    // Create Structured Buffers
    //--------------------------------------------------------------------------------------
    HRESULT CreateStructuredBuffer(
        _In_ ID3D11Device* pDevice,
        _In_ UINT uElementSize,
        _In_ UINT uCount,
        _In_reads_(uElementSize* uCount) void* pInitData,
        _Outptr_ ID3D11Buffer** ppBufOut);

    //--------------------------------------------------------------------------------------
    // Create Shader Resource View for Structured or Raw Buffers
    //--------------------------------------------------------------------------------------
    HRESULT CreateBufferSRV(_In_ ID3D11Device* pDevice, _In_ ID3D11Buffer* pBuffer, _Outptr_ ID3D11ShaderResourceView** ppSRVOut);

    //--------------------------------------------------------------------------------------
    // Create Unordered Access View for Structured or Raw Buffers
    //--------------------------------------------------------------------------------------
    HRESULT CreateBufferUAV(_In_ ID3D11Device* pDevice, _In_ ID3D11Buffer* pBuffer, _Outptr_ ID3D11UnorderedAccessView** pUAVOut);
};
