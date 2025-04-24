#include "GCCPort.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ViGEm/Client.h"

#define IS_FLAG_SET(field, flag) (field & flag == flag)
#define SET_FLAG_CONDITIONAL(field, flag, condition) field |= (flag * (condition))

namespace GCControllerEmulator
{
    inline float ApplyCalibration(const GCCPort::AxisCalibrationData& calibration, uint8_t axis)
    {
        return (float(axis) - float(calibration.Min)) * calibration.Calibration;
    }

    GCCPort::GCCPort(PVIGEM_CLIENT client)
    {
        m_TargetPad = vigem_target_x360_alloc();

        if (m_TargetPad == nullptr)
        {
            PRINT_ERR_MSG("Unable to allocate a new ViGEm Xbox 360 controller!\n");
            return;
        }

        m_Connected = false;
        m_LargeMotor = 0;
        m_SmallMotor = 0;

        VIGEM_ERROR err = vigem_target_x360_register_notification(client, m_TargetPad, &ViGEmNotification, this);

    #if DEBUG
        if (!VIGEM_SUCCESS(err))
            PRINT_ERR_MSG("Unable to register the ViGEm Xbox 360 controller for rumble!\n");
    #endif

        m_Type = Controller::Type::None;
        m_Buttons = Controller::Button::None;
        m_LeftStickX = 0;
        m_LeftStickY = 0;
        m_RightStickX = 0;
        m_RightStickY = 0;
        m_LeftTrigger = 0;
        m_RightTrigger = 0;

        m_LeftStickXCalibration.Setup(0, UINT8_MAX);
        m_LeftStickYCalibration.Setup(0, UINT8_MAX);
        m_RightStickXCalibration.Setup(0, UINT8_MAX);
        m_RightStickYCalibration.Setup(0, UINT8_MAX);
        m_LeftTriggerCalibration.Setup(0, UINT8_MAX);
        m_RightTriggerCalibration.Setup(0, UINT8_MAX);
        m_LargeMotorThreshold = 0x80;
        m_SmallMotorThreshold = 0x80;
    }

    GCCPort::~GCCPort()
    {
        if (m_TargetPad == nullptr)
            return;

        vigem_target_x360_unregister_notification(m_TargetPad);

        vigem_target_free(m_TargetPad);
        m_TargetPad = nullptr;
    }

    void GCCPort::HandlePayload(uint8_t payload[9])
    {
        m_Type = payload[0];
        m_Buttons = payload[1] | (uint16_t(payload[2]) << 8);
        m_LeftStickX = payload[3];
        m_LeftStickY = payload[4];
        m_RightStickX = payload[5];
        m_RightStickY = payload[6];
        m_LeftTrigger = payload[7];
        m_RightTrigger = payload[8];
    }

    void GCCPort::SendReport(PVIGEM_CLIENT client)
    {
        if (m_TargetPad == nullptr)
            return;

        VIGEM_ERROR error = VIGEM_ERROR_NONE;

        bool connected = (m_Type & 0xF0) != 0;
        if (m_Connected != connected)
        {
            switch (connected)
            {
                case false: 
                {
                    error = vigem_target_remove(client, m_TargetPad);

                    if (!VIGEM_SUCCESS(error))
                    {
                        PRINT_ERR_MSG("Unable to disconnect gamepad!\n");
                        return;
                    }

                    break;
                }
                case true:
                {
                    error = vigem_target_add(client, m_TargetPad);

                    if (!VIGEM_SUCCESS(error))
                    {
                        PRINT_ERR_MSG("Unable to connect gamepad!\n");
                        return;
                    }

                    break;
                }
            }

            m_Connected = connected;
        }

        if (!m_Connected) return;

        XUSB_REPORT report = { };
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_A, IS_FLAG_SET(m_Buttons, Controller::Button::A));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_B, IS_FLAG_SET(m_Buttons, Controller::Button::B));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_X, IS_FLAG_SET(m_Buttons, Controller::Button::X));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_Y, IS_FLAG_SET(m_Buttons, Controller::Button::Y));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_DPAD_LEFT, IS_FLAG_SET(m_Buttons, Controller::Button::DPadLeft));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_DPAD_RIGHT, IS_FLAG_SET(m_Buttons, Controller::Button::DPadRight));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_DPAD_DOWN, IS_FLAG_SET(m_Buttons, Controller::Button::DPadDown));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_DPAD_DOWN, IS_FLAG_SET(m_Buttons, Controller::Button::DPadUp));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_START, IS_FLAG_SET(m_Buttons, Controller::Button::StartPause));
        SET_FLAG_CONDITIONAL(report.wButtons, XUSB_GAMEPAD_RIGHT_SHOULDER, IS_FLAG_SET(m_Buttons, Controller::Button::Z));

        report.bLeftTrigger = BYTE(ApplyCalibration(m_LeftTriggerCalibration, m_LeftTrigger) * UINT8_MAX);
        report.bRightTrigger = BYTE(ApplyCalibration(m_RightTriggerCalibration, m_RightTrigger) * UINT8_MAX);
        report.sThumbLX = SHORT(ApplyCalibration(m_LeftStickXCalibration, m_LeftStickX) * UINT16_MAX - INT16_MIN);
        report.sThumbLY = SHORT(ApplyCalibration(m_LeftStickYCalibration, m_LeftStickY) * UINT16_MAX - INT16_MIN);
        report.sThumbRX = SHORT(ApplyCalibration(m_RightStickXCalibration, m_RightStickX) * UINT16_MAX - INT16_MIN);
        report.sThumbRY = SHORT(ApplyCalibration(m_RightStickYCalibration, m_RightStickY) * UINT16_MAX - INT16_MIN);

        error = vigem_target_x360_update(client, m_TargetPad, report);

    #if DEBUG
        if (VIGEM_SUCCESS(error))
        {
            PRINT_ERR_MSG("Unable to update ViGEm Xbox 360 controller!\n");
            return;
        }
    #endif
    }
    
    ControllerType GCCPort::GetType() const { return m_Type; }
    ControllerButton GCCPort::GetButtons() const { return m_Buttons; }
    ControllerAxis GCCPort::GetLeftStickXRaw() const { return m_LeftStickX; }
    ControllerAxis GCCPort::GetLeftStickYRaw() const { return m_LeftStickY; }
    ControllerAxis GCCPort::GetRightStickXRaw() const { return m_RightStickX; }
    ControllerAxis GCCPort::GetRightStickYRaw() const { return m_RightStickY; }
    ControllerAxis GCCPort::GetLeftTriggerRaw() const { return m_LeftTrigger; }
    ControllerAxis GCCPort::GetRightTriggerRaw() const { return m_RightTrigger; }
    uint8_t GCCPort::GetLargeMotor() const { return m_LargeMotor; }
    uint8_t GCCPort::GetSmallMotor() const { return m_SmallMotor; }

    ControllerAxis GCCPort::GetCalibratedLeftStickX() const
    { return uint8_t(ApplyCalibration(m_LeftStickXCalibration, m_LeftStickX) * UINT8_MAX); }
    ControllerAxis GCCPort::GetCalibratedLeftStickY() const
    { return uint8_t(ApplyCalibration(m_LeftStickYCalibration, m_LeftStickY) * UINT8_MAX); }
    ControllerAxis GCCPort::GetCalibratedRightStickX() const
    { return uint8_t(ApplyCalibration(m_RightStickXCalibration, m_RightStickX) * UINT8_MAX); }
    ControllerAxis GCCPort::GetCalibratedRightStickY() const
    { return uint8_t(ApplyCalibration(m_RightStickYCalibration, m_RightStickY) * UINT8_MAX); }
    ControllerAxis GCCPort::GetCalibratedLeftTrigger() const
    { return uint8_t(ApplyCalibration(m_LeftTriggerCalibration, m_LeftTrigger) * UINT8_MAX); }
    ControllerAxis GCCPort::GetCalibratedRightTrigger() const
    { return uint8_t(ApplyCalibration(m_RightTriggerCalibration, m_RightTrigger) * UINT8_MAX); }
    bool GCCPort::GetRumble() const
    { return (m_LargeMotor > m_LargeMotorThreshold) || (m_SmallMotor > m_SmallMotorThreshold); }

    void GCCPort::CalibrateLeftStickX(uint8_t min, uint8_t max) { m_LeftStickXCalibration.Setup(min, max); }
    void GCCPort::CalibrateLeftStickY(uint8_t min, uint8_t max) { m_LeftStickYCalibration.Setup(min, max); }
    void GCCPort::CalibrateRightStickX(uint8_t min, uint8_t max) { m_RightStickXCalibration.Setup(min, max); }
    void GCCPort::CalibrateRightStickY(uint8_t min, uint8_t max) { m_RightStickYCalibration.Setup(min, max); }
    void GCCPort::CalibrateLeftTrigger(uint8_t min, uint8_t max) { m_LeftTriggerCalibration.Setup(min, max); }
    void GCCPort::CalibrateRightTrigger(uint8_t min, uint8_t max) { m_RightTriggerCalibration.Setup(min, max); }
    void GCCPort::CalibrateRumble(uint8_t largeMotorThreshold, uint8_t smallMotorThreshold) { m_LargeMotorThreshold = largeMotorThreshold; m_SmallMotorThreshold = smallMotorThreshold; }

    void GCCPort::ViGEmNotification(PVIGEM_CLIENT client, PVIGEM_TARGET target, uint8_t largeMotor, uint8_t smallMotor, uint8_t ledNumber, void *userData)
    {
        GCCPort* port = (GCCPort*)userData;

        port->m_LargeMotor = largeMotor;
        port->m_SmallMotor = smallMotor;
    }

    void GCCPort::AxisCalibrationData::Setup(uint8_t min, uint8_t max)
    {
        if (min == max)
        {
            Min = min;
            Calibration = 0.0f;
            return;
        }

        Min = min;
        Calibration = 1.0f / (float(max) - float(min));
    }
}
