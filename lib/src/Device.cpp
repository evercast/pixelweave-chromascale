#include "Device.h"

#include "VulkanDevice.h"

namespace PixelWeave
{
ResultValue<Device*> Device::Create()
{
    return VulkanDevice::Create();
}
}  // namespace PixelWeave
