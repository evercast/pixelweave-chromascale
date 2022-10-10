#include "ResourceLoader.h"

#include <algorithm>

#include <Windows.h>

#include "ShaderResources.h"

namespace PixelWeave
{

Resource ResourceLoader::Load(const Resource::Id& resourceId)
{
    uint32_t actualId = 0;
    switch (resourceId) {
        case Resource::Id::ComputeShader:
            actualId = PW_RESOURCE_COMPUTE_SHADER;
            break;
    }
    HMODULE module = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&ResourceLoader::Load,
        &module);
    HRSRC resource = FindResource(module, MAKEINTRESOURCE(actualId), RT_RCDATA);
    if (resource != nullptr) {
        size_t bufferSize = SizeofResource(module, resource);
        HGLOBAL data = LoadResource(module, resource);
        if (data != nullptr && bufferSize != 0) {
            uint8_t* buffer = reinterpret_cast<uint8_t*>(LockResource(data));

            Resource result;
            result.size = bufferSize;
            result.buffer = new uint8_t[bufferSize];
            std::copy_n(buffer, bufferSize, result.buffer);
            return result;
        }
    }
    return {0, nullptr};
}

void ResourceLoader::Cleanup(Resource& resource) {
    delete[] resource.buffer;
    resource.buffer = nullptr;
    resource.size = 0;
}

}  // namespace PixelWeave
