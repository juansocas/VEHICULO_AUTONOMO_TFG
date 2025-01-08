#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <pluginlib/class_list_macros.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/state.hpp>
#include <vector>
#include <string>

#include "jsc_mecanum_hardware/jsc_mecanum_hardware.hpp"
#define HDLC_FRAME_BOUNDRY_FLAG 0x7E

PLUGINLIB_EXPORT_CLASS(
    jsc::jsc_mecanum_bot::hardware::MecanumbotHardware,
    hardware_interface::SystemInterface
)

using namespace jsc::jsc_mecanum_bot::hardware;

hardware_interface::CallbackReturn MecanumbotHardware::on_init(const hardware_interface::HardwareInfo & hardware_info)
{
    hardware_interface::CallbackReturn baseResult = hardware_interface::SystemInterface::on_init(hardware_info);
    if (baseResult != hardware_interface::CallbackReturn::SUCCESS) {
        return baseResult;
    }

    // YO

    //hw_start_sec = std::stod(info_.hardware_parameters[""])

    //

    serial_port_name_ = info_.hardware_parameters["serial_port"];
    motor_ids_.resize(info_.joints.size());
    position_states_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
    velocity_states_.resize(info_.joints.size(), 0.0);
    velocity_commands_.resize(info_.joints.size(), 0.0);
    velocity_commands_saved_.resize(info_.joints.size(), 0.0);
    
    for (hardware_interface::ComponentInfo & joint : info_.joints)
    {
        if (joint.parameters["motor_id"].empty()) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Motor id not defined for join %s", joint.name.c_str());
            return hardware_interface::CallbackReturn::ERROR;
        }
        if (joint.command_interfaces.size() != 1) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Invalid number of command interfaces (expected: 1)");
            return hardware_interface::CallbackReturn::ERROR;
        }
        if (joint.command_interfaces[0].name != hardware_interface::HW_IF_VELOCITY) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Invalid joint command interface 0 type (expected: velocity)");
            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.state_interfaces.size() != 2) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Invalid number of state interfaces (expected: 2)");
            return hardware_interface::CallbackReturn::ERROR;
        }
        if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Invalid joint state interface 0 type (expected: position)");
            return hardware_interface::CallbackReturn::ERROR;
        }
        if (joint.state_interfaces[1].name != hardware_interface::HW_IF_VELOCITY) {
            RCLCPP_FATAL(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Invalid joint state interface 1 type (expected: velocity)");
            return hardware_interface::CallbackReturn::ERROR;
        }
    }

    for (size_t i = 0; i < info_.joints.size(); i++) {
        motor_ids_[i] = (uint8_t)std::stoi(info_.joints[i].parameters["motor_id"]);
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "%s mapped to motor %d", info_.joints[i].name.c_str(), motor_ids_[i]);
    }


    // SI ESTA TODO CORRECTO RETORNA SUCCESS
    return hardware_interface::CallbackReturn::SUCCESS;
}

// FUNCION ACTUALIZAR LOS ESTADOS DE LA INTERFAZ 

std::vector<hardware_interface::StateInterface> MecanumbotHardware::export_state_interfaces()
{
    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "export_state_interfaces");

    std::vector<hardware_interface::StateInterface> state_interfaces;

    
    for (size_t i = 0; i < info_.joints.size(); i++) {
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Adding position state interface: %s", info_.joints[i].name.c_str());
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                info_.joints[i].name, hardware_interface::HW_IF_POSITION, &position_states_[i]
            )
        );
    }
    for (size_t i = 0; i < info_.joints.size(); i++) {
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Adding velocity state interface: %s", info_.joints[i].name.c_str());
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &velocity_states_[i]
                
            )
        );
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Initial velocity for %s: %.2f", info_.joints[i].name.c_str(), velocity_states_[i]);
    }
    return state_interfaces;
}

// FUNCION ACTUALIZAR LOS COMANDOS DE LA INTERFAZ 

std::vector<hardware_interface::CommandInterface> MecanumbotHardware::export_command_interfaces()
{
    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "export_command_interfaces");

    std::vector<hardware_interface::CommandInterface> command_interfaces;
    for (size_t i = 0; i < info_.joints.size(); i++) {
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Adding velocity command interface: %s", info_.joints[i].name.c_str());
        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(
                info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &velocity_commands_[i]
            )
        );
    }
    return command_interfaces;
}

hardware_interface::CallbackReturn MecanumbotHardware::on_activate(const rclcpp_lifecycle::State & previous_state)
{
    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware starting ...");

    for (size_t i = 0; i < info_.joints.size(); i++) {
        if (std::isnan(position_states_[i])) {
            position_states_[i] = 0.0f;
        }
        if (std::isnan(velocity_states_[i])) {
            velocity_states_[i] = 0.0f;
        }
        if (std::isnan(velocity_commands_[i])) {
            velocity_commands_[i] = 0.0f;
        }
        velocity_commands_saved_[i] = velocity_commands_[i];
    }

    serial_port_ = std::make_shared<MecanumbotSerialPort>();
    if (serial_port_->open(serial_port_name_) != return_type::SUCCESS) {
        RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware no conectado al puerto serie");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware started");
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MecanumbotHardware::on_deactivate(const rclcpp_lifecycle::State & previous_state)
{
    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware stopping ...");

    if (serial_port_->is_open()) {
        serial_port_->close();
        serial_port_.reset();
    }

    RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware stopped");
    return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

hardware_interface::return_type MecanumbotHardware::read(const rclcpp::Time & time, const rclcpp::Duration & period)
{
    if (!serial_port_->is_open()) {
        RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "JSC_Mecanum hardware no conectado al puerto serie");
        return hardware_interface::return_type::ERROR;
    }

    std::vector<SerialHdlcFrame> frames;
    serial_port_->read_frames(frames);

    // Procesar cada frame
    for (const auto &frame : frames) {
        std::string frame_data(frame.data, frame.data + frame.length);
        RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanumbot_Hardware"),"Frame Data: %s, Size: %lu", frame_data.c_str(), frame_data.size());
        
        size_t vel_start = frame_data.find("V:");

        if (vel_start != std::string::npos) {
            // Extraer y procesar solo las velocidades
            std::string velocities_str = frame_data.substr(vel_start + 2);  
            std::vector<std::string> vel_tokens = split(velocities_str, ':');
            RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanumbot_Hardware"), "Procesando velocidades...");

            // Verificar que tenemos 4 valores para las velocidades
            if (vel_tokens.size() == 4) {
                for (size_t j = 0; j < 4; j++) {
                    velocity_states_[j] = std::stof(vel_tokens[j]);
                    RCLCPP_INFO(rclcpp::get_logger("MecanumbotHardware"),
                                "Updated motor %zu with velocity %.2f", 
                                j, velocity_states_[j]);
                }
            } else {
                RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Frame malformada: %s", frame_data.c_str());
            }
        } else {
            RCLCPP_WARN(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Frame formada incorrectamente: %s", frame_data.c_str());
        }
    }
    return hardware_interface::return_type::OK;
}
hardware_interface::return_type MecanumbotHardware::write(const rclcpp::Time & time, const rclcpp::Duration & period)
{
    bool send_command = false;
    std::ostringstream command_stream;
    command_stream << "M:";

    for (size_t i = 0; i < info_.joints.size(); i++) {
        if (velocity_commands_[i] != velocity_commands_saved_[i]) {
            send_command = true;
        }
        command_stream << static_cast<int>(velocity_commands_[i]);
        if (i < info_.joints.size() - 1) {
            command_stream << ":";
        }
        //RCLCPP_INFO(rclcpp::get_logger("jsc_mecanum_Hardware"), "Comando de velocidad para el motor %zu: %s", i, std::to_string(velocity_commands_[i]).c_str());
        velocity_commands_saved_[i] = velocity_commands_[i];
    }

    if (send_command && serial_port_->is_open()) {
        std::string command = command_stream.str();
        RCLCPP_INFO(rclcpp::get_logger("JSC_Mecanum_Hardware"), "Enviando command: %s", command.c_str());
        serial_port_->write_frame(reinterpret_cast<const uint8_t*>(command.c_str()), command.size());
    }
    return hardware_interface::return_type::OK;
}

