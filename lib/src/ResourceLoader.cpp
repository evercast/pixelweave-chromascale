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
        {
            resourceName = ResourceName { CFSTR("convert.comp"), CFSTR("spv") };
        }
        break;
    }
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef urlRef = CFBundleCopyResourceURL(mainBundle, resourceName.name, resourceName.extension, NULL);
    CFStringRef urlString = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
    std::string path = CFStringGetCStringPtr(urlString, kCFStringEncodingUTF8);
    FILE* file = fopen(path.c_str(), "rb");
    if (file != nullptr) {
        fseek(file, 0L, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0L, SEEK_SET);
        Resource result;
        result.size = fileSize;
        result.buffer = new uint8_t[fileSize];
        fread(result.buffer, fileSize, sizeof(uint8_t), file);
        fclose(file);
        return result;
    }
}
#endif

Resource ResourceLoader::Load(const Resource::Id& resourceId)
{
#if defined(PW_PLATFORM_WINDOWS)
    return LoadWindows(resourceId);
#elif defined(PW_PLATFORM_MACOS)
    return LoadMac(resourceId);
#endif
}

void ResourceLoader::Cleanup(Resource& resource)
{
    delete[] resource.buffer;
    resource.buffer = nullptr;
    resource.size = 0;
}

}  // namespace PixelWeave
