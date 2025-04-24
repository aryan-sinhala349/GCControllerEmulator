#pragma once

#include "Base.h"
#include "Controller.h"

//Forward declarations
typedef struct _VIGEM_CLIENT_T* PVIGEM_CLIENT;
typedef struct _VIGEM_TARGET_T* PVIGEM_TARGET;

namespace GCControllerEmulator
{
    class GC_CONTROLLER_EMULATOR_PUBLIC GCCPort
    {
    public:
        struct AxisCalibrationData
        {
            ControllerAxis Min;
            float Calibration;

            inline void Setup(uint8_t min, uint8_t max);
        };

        GCCPort() = default;
        GCCPort(PVIGEM_CLIENT client);
        ~GCCPort();

        void HandlePayload(uint8_t payload[9]);
        void SendReport(PVIGEM_CLIENT client);

        ControllerType GetType() const;
        ControllerButton GetButtons() const;
        ControllerAxis GetLeftStickXRaw() const;
        ControllerAxis GetLeftStickYRaw() const;
        ControllerAxis GetRightStickXRaw() const;
        ControllerAxis GetRightStickYRaw() const;
        ControllerAxis GetLeftTriggerRaw() const;
        ControllerAxis GetRightTriggerRaw() const;
        uint8_t GetLargeMotor() const;
        uint8_t GetSmallMotor() const;

        ControllerAxis GetCalibratedLeftStickX() const;
        ControllerAxis GetCalibratedLeftStickY() const;
        ControllerAxis GetCalibratedRightStickX() const;
        ControllerAxis GetCalibratedRightStickY() const;
        ControllerAxis GetCalibratedLeftTrigger() const;
        ControllerAxis GetCalibratedRightTrigger() const;
        bool GetRumble() const;

        void CalibrateLeftStickX(uint8_t min, uint8_t max);
        void CalibrateLeftStickY(uint8_t min, uint8_t max);
        void CalibrateRightStickX(uint8_t min, uint8_t max);
        void CalibrateRightStickY(uint8_t min, uint8_t max);
        void CalibrateLeftTrigger(uint8_t min, uint8_t max);
        void CalibrateRightTrigger(uint8_t min, uint8_t max);
        void CalibrateRumble(uint8_t largeMotorThreshold, uint8_t smallMotorThreshold);

    private:
        //ViGEm data
        PVIGEM_TARGET m_TargetPad;
        bool m_Connected;
        uint8_t m_LargeMotor;
        uint8_t m_SmallMotor;

        //Raw controller data
        ControllerType m_Type;
        ControllerButton m_Buttons;
        ControllerAxis m_LeftStickX;
        ControllerAxis m_LeftStickY;
        ControllerAxis m_RightStickX;
        ControllerAxis m_RightStickY;
        ControllerAxis m_LeftTrigger;
        ControllerAxis m_RightTrigger;

        //Calibration data
        AxisCalibrationData m_LeftStickXCalibration;
        AxisCalibrationData m_LeftStickYCalibration;
        AxisCalibrationData m_RightStickXCalibration;
        AxisCalibrationData m_RightStickYCalibration;
        AxisCalibrationData m_LeftTriggerCalibration;
        AxisCalibrationData m_RightTriggerCalibration;
        uint8_t m_LargeMotorThreshold;
        uint8_t m_SmallMotorThreshold;

        static void ViGEmNotification(PVIGEM_CLIENT client, PVIGEM_TARGET target, uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber, void* userData);
    };
}
