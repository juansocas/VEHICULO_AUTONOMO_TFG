#ifndef __DEBICT_JSC_MECANUM_HARDWARE__JSC_MECANUM_HARDWARE_H__
#define __DEBICT_JSC_MECANUM_HARDWARE__JSC_MECANUM_HARDWARE_H__

//#include <hardware_interface/base_interface.hpp>
#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/visibility_control.h>
#include <rclcpp/macros.hpp>
#include <vector>

#include "jsc_mecanum_hardware/jsc_mecanum_serial_port.hpp"
#include "jsc_mecanum_hardware/jsc_mecanum_hardware_compiler.h"

namespace jsc
{
    namespace jsc_mecanum_bot
    {
        namespace hardware
        {
            enum class DeviceCommand : uint8_t {
                MotorSetDuty = 0x01,
                MotorBrake   = 0x02,
                MotorStop    = 0x03,
            };

            enum class DeviceMotorDirection : uint8_t {
                None    = 0,
                Forward = 1,
                Reverse = 2,
            };
            
            class MecanumbotHardware
                : public hardware_interface::SystemInterface
            {

            
            // struct Config {
            //     std::string fl_wheel_name = "";
            //     std::string fr_wheel_name = "";
            //     std::string rl_wheel_name = "";
            //     std::string rr_wheel_name = "";
            //     float loop_rate = 0.0;
            //     std::string device = "";
            //     int baud_rate = 0;
            //     int timeout_ms = 0;
            //     int enc_counts_per_rev = 0;
            // }


            public:
                RCLCPP_SHARED_PTR_DEFINITIONS(MecanumbotHardware)

                JSC_MECANUM_HARDWARE_PUBLIC
                virtual hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo & hardware_info) override;

                //on_configure;
                //on_cleanup;
                //on_activate;
                //on_deactivate;
                //on_shutdown;
                //on_error;
                
                JSC_MECANUM_HARDWARE_PUBLIC
                virtual std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
                
                JSC_MECANUM_HARDWARE_PUBLIC
                virtual std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;
                
                JSC_MECANUM_HARDWARE_PUBLIC
                virtual hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;
                
                JSC_MECANUM_HARDWARE_PUBLIC
                virtual hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;
                
                //Funcion para leer del Hardware
        
                JSC_MECANUM_HARDWARE_PUBLIC
                virtual hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) override;
                
                //Funcion para escribir al Hardware

                JSC_MECANUM_HARDWARE_PUBLIC
                virtual hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override;

            private:

                // VARIABLES QUE CONTIENE LOS VALORES DE ESTADOS Y COMANDOS
                std::vector<uint8_t> motor_ids_;
                std::vector<double> position_states_;
                std::vector<double> velocity_states_;
                std::vector<double> velocity_commands_;
                std::vector<double> velocity_commands_saved_;
                std::shared_ptr<MecanumbotSerialPort> serial_port_;
                std::string serial_port_name_;

                // ArduinoComms comms_;
                //Config cfg_;

            };
        }
    }
}

#endif // __JSC_MECANUM_HARDWARE__MECANUM_HARDWARE_H__