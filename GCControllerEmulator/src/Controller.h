#pragma once

#include <cstdint>

namespace GCControllerEmulator
{
    using ControllerType = std::uint8_t;
    using ControllerButton = std::uint16_t;
    using ControllerAxis = std::uint8_t;
    
    namespace Controller
    {
        namespace Type
        {
            enum : ControllerType
            {
                None = 0,
            
                ExtraPower = 0x04,
            
                Standard = 0x10,
                Wavebird = 0x20,
            };
        }
    
        namespace Button
        {
            enum : ControllerButton
            {
                None = 0,
            
                A           = 1 <<  0,
                B           = 1 <<  1,
                X           = 1 <<  2,
                Y           = 1 <<  3,
                DPadLeft    = 1 <<  4,
                DPadRight   = 1 <<  5,
                DPadDown    = 1 <<  6,
                DPadUp      = 1 <<  7,
                StartPause  = 1 <<  8,
                Z           = 1 <<  9,
                R           = 1 << 10,
                L           = 1 << 11,
            };
        }
    }
}
