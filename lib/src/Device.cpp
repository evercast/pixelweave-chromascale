#include "Device.h"

#include "VulkanDevice.h"

namespace Pixelweave
{
ResultValue<Device*> Device::Create()
{
    return VulkanDevice::Create();
}
}  // namespace Pixelweave
