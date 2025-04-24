#pragma once

#include "Base.h"
#include "GCCPort.h"

struct libusb_device;
struct libusb_device_handle;
struct libusb_transfer;

namespace GCControllerEmulator
{
    class GC_CONTROLLER_EMULATOR_PUBLIC GCCAdapter
    {
    public:
        GCCAdapter(libusb_device* device, PVIGEM_CLIENT client);
        ~GCCAdapter();

        void Update(PVIGEM_CLIENT client);

    private:
        GCCPort m_Ports[4] = { };

        libusb_device* m_Device = nullptr;
        libusb_device_handle* m_Handle = nullptr;
        bool m_KernelDriverAttached = false;

        uint8_t m_InBuffer[37] = { 0 };
        uint8_t m_RumbleBuffer[5] = { 0 };

        libusb_transfer* m_InTransfer = nullptr;
        libusb_transfer* m_RumbleTransfer = nullptr;

        static void OnInputCompleted(libusb_transfer* transfer);
        static void OnRumbleCompleted(libusb_transfer* transfer);
    };
}
