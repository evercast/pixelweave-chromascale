#define PIXEL_WEAVE_DEVICE_IMPL
#include "Device.h"

#include "VulkanDevice.h"

namespace PixelWeave
{
ResultValue<std::shared_ptr<Device>> Device::Create()
{
    return VulkanDevice::Create();
}
}  // namespace PixelWeave
