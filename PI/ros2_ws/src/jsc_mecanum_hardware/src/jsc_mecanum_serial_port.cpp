#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <rclcpp/rclcpp.hpp>

#include "jsc_mecanum_hardware/jsc_mecanum_serial_port.hpp"

#define HDLC_FRAME_BOUNDRY_FLAG     0x7E
#define HDLC_ESCAPE_FLAG            0x7D
#define HDLC_ESCAPE_XOR             0x20
#define HDLC_CRC_INIT_VALUE         0xFFFF

using namespace jsc::jsc_mecanum_bot::hardware;

MecanumbotSerialPort::MecanumbotSerialPort()
    : serial_port_(-1)
    , rx_frame_length_(0)
    , rx_frame_crc_(HDLC_CRC_INIT_VALUE)
    , rx_frame_escape_(false)
    , tx_frame_length_(0)
    , tx_frame_crc_(HDLC_CRC_INIT_VALUE)
{

}


MecanumbotSerialPort::~MecanumbotSerialPort()
{
    close();
}

return_type MecanumbotSerialPort::open(const std::string & port_name)
{
    serial_port_ = ::open(port_name.c_str(), O_RDWR | O_NOCTTY);

    if (serial_port_ < 0) {
        RCLCPP_ERROR(rclcpp::get_logger("JSCMecanumSerialPort"), "Failed to open serial port: %s (%d)", strerror(errno), errno);
        return return_type::ERROR; 
    }
    
    RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "Opening port: %s", port_name.c_str());


    struct termios tty_config{};
    if (::tcgetattr(serial_port_, &tty_config) != 0) {
        RCLCPP_ERROR(rclcpp::get_logger("JSCMecanumSerialPort"), "Failed to get serial port configuration: %s (%d)", strerror(errno), errno);
        close();
        return return_type::ERROR;
    }

    memset(&tty_config, 0, sizeof(termios));
    tty_config.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
    tty_config.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty_config.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty_config.c_cflag &= ~CSIZE;    // Clear bits per byte
    tty_config.c_cflag |=  CS8;      // 8 bit per byte
    tty_config.c_iflag = IGNPAR;
    tty_config.c_oflag = OPOST;
    tty_config.c_lflag = 0;
    tty_config.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty_config.c_cc[VMIN] = 0;
    tcflush(serial_port_, TCIFLUSH);

    /*
    if (::cfsetispeed(&tty_config, B9600) != 0 || ::cfsetospeed(&tty_config, B9600) != 0) {
        fprintf(stderr, "Failed to set serial port speed: %s (%d)\n", strerror(errno), errno);
        close();
        return return_type::ERROR;
    }
    */

    if (::tcsetattr(serial_port_, TCSANOW, &tty_config) != 0) {
        RCLCPP_ERROR(rclcpp::get_logger("JSCMecanumSerialPort"), "Failed to set serial port configuration: %s (%d)", strerror(errno), errno);
        close();
        return return_type::ERROR;
    }

    return return_type::SUCCESS;
}

return_type MecanumbotSerialPort::close()
{
    if (is_open()) {
        ::close(serial_port_);
        serial_port_ = -1;
    }
    return return_type::SUCCESS;
}

return_type MecanumbotSerialPort::read_frames(std::vector<SerialHdlcFrame>& frames)
{
    // Read data from the serial port
    const ssize_t num_bytes = ::read(serial_port_, rx_buffer_, 256);

    if (num_bytes == -1) {
        RCLCPP_ERROR(rclcpp::get_logger("JSCMecanumSerialPort"), "Failed to read serial port data: %s (%d)", strerror(errno), errno);
        return return_type::ERROR;
    }

    for (ssize_t i = 0; i < num_bytes; i++) {
        
        decode_byte(rx_buffer_[i], frames);
    }

    return return_type::SUCCESS;
}

return_type MecanumbotSerialPort::write_frame(const uint8_t* data, size_t size)
{
    if (!is_open()) {
        return return_type::ERROR;
    }

    tx_frame_length_ = 0;
    tx_frame_crc_ = HDLC_CRC_INIT_VALUE;
    tx_frame_buffer_[tx_frame_length_++] = HDLC_FRAME_BOUNDRY_FLAG;

    for (size_t i = 0; i < size; i++) {
        tx_frame_crc_ = crc_update(tx_frame_crc_, data[i]);
        encode_byte(data[i]);
    }
    // Añadir CRC al frame
    encode_byte((tx_frame_crc_ & 0xFF));
    encode_byte((tx_frame_crc_ >> 8) & 0xFF);

    // Añadir delimitador de fin
    tx_frame_buffer_[tx_frame_length_++] = HDLC_FRAME_BOUNDRY_FLAG;


    // Enviar el frame completo por el puerto serie y verificar que no haya errores
    if (::write(serial_port_, tx_frame_buffer_, tx_frame_length_) == -1) {
        return return_type::ERROR;
    }
    return return_type::SUCCESS;
}

bool MecanumbotSerialPort::is_open() const
{
    return serial_port_ >= 0;
}

void MecanumbotSerialPort::encode_byte(uint8_t data)
{
    if (data == HDLC_ESCAPE_FLAG || data == HDLC_FRAME_BOUNDRY_FLAG) {
        tx_frame_buffer_[tx_frame_length_++] = HDLC_ESCAPE_FLAG;
        data ^= HDLC_ESCAPE_XOR;
    }
    tx_frame_buffer_[tx_frame_length_++] = data;
}

void MecanumbotSerialPort::decode_byte(uint8_t data, std::vector<SerialHdlcFrame>& frames)
{
    
    if (data == HDLC_FRAME_BOUNDRY_FLAG) {
        if (rx_frame_length_ > 2) { // Verifica que al menos tenemos un mínimo de datos, incluyendo el CRC
            // Extrae el CRC recibido desde el final del frame
            uint16_t received_crc = (rx_frame_buffer_[rx_frame_length_ - 1] << 8) | rx_frame_buffer_[rx_frame_length_ - 2];
            
            // Log de verificación para CRC y contenido de la trama
            RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "Contenido de frame recibido:");
            
            RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "CRC Calculado: %04X, CRC Recibido: %04X", rx_frame_crc_, received_crc);

            // Verifica el CRC calculado con el recibido
            if (rx_frame_crc_ == received_crc) {
                SerialHdlcFrame frame;
                memcpy(frame.data, rx_frame_buffer_, rx_frame_length_ - 2); // Excluye los bytes de CRC
                frame.length = rx_frame_length_ - 2;
                frames.push_back(frame);
                RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "Frame válido recibido y procesado.");
            } else {
                RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "CRC inválido");
            }
        } else {
            RCLCPP_WARN(rclcpp::get_logger("JSCMecanumSerialPort"), "Frame incompleto");
        }

        // Reinicia el frame para el siguiente paquete
        rx_frame_length_ = 0;
        rx_frame_crc_ = HDLC_CRC_INIT_VALUE;
        return;
    }

    // Maneja el byte de escape si está presente
    if (data == HDLC_ESCAPE_FLAG) {
        rx_frame_escape_ = true;
        return;
    }
    if (rx_frame_escape_) {
        data ^= HDLC_ESCAPE_XOR;
        rx_frame_escape_ = false;
    }

    // Almacena el byte en el buffer y actualiza el CRC
    if (rx_frame_length_ < JSC_MECANUM_BOT_SERIAL_SERIAL_FRAME_MAX_SIZE) {
        rx_frame_buffer_[rx_frame_length_++] = data;
        if (rx_frame_length_ > 2) { // Evita actualizar CRC si no hay suficiente longitud
            rx_frame_crc_ = crc_update(rx_frame_crc_, rx_frame_buffer_[rx_frame_length_ - 3]);
        }
    }
}

uint16_t MecanumbotSerialPort::crc_update(uint16_t crc, uint8_t data)
{
    data ^= (uint8_t)(crc & 0xFF);
    data ^= (data << 4);
    return ((((uint16_t)data << 8) | ((uint8_t)(crc >> 8) & 0xFF)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}