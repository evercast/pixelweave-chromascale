#include "ResourceLoader.h"

#include <algorithm>
#include <string>

#ifdef PW_PLATFORM_WINDOWS
#include <Windows.h>
#include "ShaderResources.h"
#endif

#ifdef PW_PLATFORM_MACOS
#include <CoreFoundation/CFBundle.h>
#endif

#ifdef PW_PLATFORM_LINUX
extern "C" {
#include "../../thirdparty/incbin/incbin.h"
}

INCBIN(ComputeShader, "../shaders/convert.comp");
#endif

namespace PixelWeave
{

#ifdef PW_PLATFORM_WINDOWS
Resource LoadWindows(const Resource::Id& resourceId)
{
    uint32_t actualId = 0;
    switch (resourceId) {
        case Resource::Id::ComputeShader:
            actualId = PW_RESOURCE_COMPUTE_SHADER;
            break;
        default:
            return {nullptr, 0};
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
            auto resourceBuffer = reinterpret_cast<uint8_t*>(LockResource(data));
            auto buffer = new uint8_t[bufferSize];
            std::copy_n(resourceBuffer, bufferSize, buffer);
            return {buffer, bufferSize};
        }
    }
    return {nullptr, 0};
}
#endif

#ifdef PW_PLATFORM_MACOS
Resource LoadMac(const Resource::Id& resourceId)
{
    struct ResourceName {
        CFStringRef name = CFSTR("");
        CFStringRef extension = CFSTR("");
    };

    ResourceName resourceName;
    switch (resourceId) {
        case Resource::Id::ComputeShader:
            resourceName = ResourceName{CFSTR("convert"), CFSTR("comp")};
            break;
        default:
            return {nullptr, 0};
    }
    CFBundleRef libraryBundle = CFBundleGetBundleWithIdentifier(CFSTR(PW_BUNDLE_ID));
    CFURLRef urlRef = CFBundleCopyResourceURL(libraryBundle, resourceName.name, resourceName.extension, NULL);
    CFStringRef urlString = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
    std::string path = CFStringGetCStringPtr(urlString, kCFStringEncodingUTF8);
    FILE* file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return {nullptr, 0};
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);
    auto buffer = new uint8_t[fileSize];
    fread(buffer, fileSize, sizeof(uint8_t), file);
    fclose(file);
    return {buffer, fileSize};
}
#endif

#ifdef PW_PLATFORM_LINUX
Resource LoadLinux(const Resource::Id& resourceId)
{
    const uint8_t* data = nullptr;
    size_t size = 0;
    switch (resourceId) {
        case Resource::Id::ComputeShader:
            data = gComputeShaderData;
            size = gComputeShaderSize;
            break;
        default:
            return {nullptr, 0};
    }
    return {data, size};
}
#endif

Resource ResourceLoader::Load(const Resource::Id& resourceId)
{
#if defined(PW_PLATFORM_WINDOWS)
    return LoadWindows(resourceId);
#elif defined(PW_PLATFORM_MACOS)
    return LoadMac(resourceId);
#elif defined(PW_PLATFORM_LINUX)
    return LoadLinux(resourceId);
#endif
}

void ResourceLoader::CleanUp(Resource& resource)
{
#ifndef PW_PLATFORM_LINUX
    delete[] resource.buffer;
#endif
    resource.buffer = nullptr;
    resource.size = 0;
}

}  // namespace PixelWeave
