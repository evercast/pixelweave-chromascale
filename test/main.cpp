#include "Device.h"

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {}
	return 0;
}