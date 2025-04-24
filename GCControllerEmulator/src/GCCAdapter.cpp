#include "GCCAdapter.h"

#include "libusb.h"
#include "ViGEm/Client.h"

#include <iostream>

#define GCC_ADAPTER_ENDPOINT_IN 0x81
#define GCC_ADAPTER_ENDPOINT_OUT 0x02

namespace GCControllerEmulator
{
    GCCAdapter::GCCAdapter(libusb_device* device, PVIGEM_CLIENT client)
        : m_Device(device)
    {
        int success = libusb_open(device, &m_Handle);
        if (success != LIBUSB_SUCCESS)
        {
            PRINT_ERR_MSG("Unable to open GameCube Adapter: %s\n", libusb_strerror(success));
            return;
        }

        if (libusb_kernel_driver_active(m_Handle, 0) == 1)
        {
            success = libusb_detach_kernel_driver(m_Handle, 0);
            if (success != LIBUSB_SUCCESS)
            {
                PRINT_ERR_MSG("Unable to detach GameCube Adapter from kernel driver: %s\n", libusb_strerror(success));
                return;
            }
            m_KernelDriverAttached = true;
        }

        success = libusb_claim_interface(m_Handle, 0);
        if (success != LIBUSB_SUCCESS)
        {
            PRINT_ERR_MSG("Unable to claim the GameCube Adapter interface: %s\n", libusb_strerr(success));
            return;
        }

        for (uint8_t i = 0; i < 4; ++i)
            m_Ports[i] = GCCPort(client);

        m_InTransfer = libusb_alloc_transfer(0);
        if (!m_InTransfer)
        {
            PRINT_ERR_MSG("Unable to allocate enough memory for the input transfer buffer!\n");
            return;
        }
        libusb_fill_interrupt_transfer(m_InTransfer, m_Handle, GCC_ADAPTER_ENDPOINT_IN, m_InBuffer, sizeof(m_InBuffer), OnInputCompleted, this, 0);

        m_RumbleTransfer = libusb_alloc_transfer(0);
        if (!m_RumbleTransfer)
        {
            PRINT_ERR_MSG("Unable to allocate enough memory for the rumble transfer buffer!\n");
            return;
        }
        libusb_fill_interrupt_transfer(m_RumbleTransfer, m_Handle, GCC_ADAPTER_ENDPOINT_OUT, m_RumbleBuffer, sizeof(m_RumbleBuffer), OnRumbleCompleted, this, 0);
    }

    GCCAdapter::~GCCAdapter()
    {
        if (m_RumbleTransfer)
        {
            libusb_cancel_transfer(m_RumbleTransfer);

            uint8_t rumblePayload[5] = { 0x11, 0, 0, 0, 0 };
            int bytesTransferred = 0;
            libusb_interrupt_transfer(m_Handle, GCC_ADAPTER_ENDPOINT_OUT, rumblePayload, sizeof(rumblePayload), &bytesTransferred, 0);

            libusb_free_transfer(m_RumbleTransfer);
            m_RumbleTransfer = nullptr;
        }        

        if (m_InTransfer)
        {
            libusb_cancel_transfer(m_InTransfer);
            libusb_free_transfer(m_InTransfer);
            m_InTransfer = nullptr;
        }

        if (m_Handle)
        {
            libusb_release_interface(m_Handle, 0);
            if (m_KernelDriverAttached) libusb_attach_kernel_driver(m_Handle, 0);
            libusb_close(m_Handle);
            m_Handle = nullptr;
            m_Device = nullptr;
        }
    }

    void GCCAdapter::Update(PVIGEM_CLIENT client)
    {
        m_Ports[0].SendReport(client);
        m_Ports[1].SendReport(client);
        m_Ports[2].SendReport(client);
        m_Ports[3].SendReport(client);

        libusb_submit_transfer(m_InTransfer);

        m_RumbleBuffer[0] = 0x11;
        m_RumbleBuffer[1] = m_Ports[0].GetRumble();
        m_RumbleBuffer[2] = m_Ports[1].GetRumble();
        m_RumbleBuffer[3] = m_Ports[2].GetRumble();
        m_RumbleBuffer[4] = m_Ports[3].GetRumble();

        libusb_submit_transfer(m_RumbleTransfer);
    }

    void GCCAdapter::OnInputCompleted(libusb_transfer* transfer)
    {
        GCCAdapter* adapter = (GCCAdapter*)transfer->user_data;

        adapter->m_Ports[0].HandlePayload(&transfer->buffer[ 1]);
        adapter->m_Ports[1].HandlePayload(&transfer->buffer[10]);
        adapter->m_Ports[2].HandlePayload(&transfer->buffer[19]);
        adapter->m_Ports[3].HandlePayload(&transfer->buffer[28]);
    }

    void GCCAdapter::OnRumbleCompleted(libusb_transfer* transfer)
    {
        //Intentionally left empty
    }
}
