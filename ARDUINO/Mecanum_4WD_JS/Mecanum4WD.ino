#include <Wire.h>
#include "PS2X_lib.h"
#include <Arduino.h>
#include "QGPMaker_MotorShield.h"
#include "QGPMaker_Encoder.h"

// Codificación y decodificación
#define HDLC_FRAME_BOUNDRY_FLAG  0x7E
#define HDLC_ESCAPE_FLAG         0x7D
#define HDLC_ESCAPE_XOR          0x20
#define HDLC_CRC_INIT_VALUE      0xFFFF

uint8_t rx_buffer[256];
uint16_t rx_crc = HDLC_CRC_INIT_VALUE;
bool escape_next = false;
int rx_length = 0;

//Variable para tiempo de cada byte recibido
unsigned long last_byte_time = 0;
// Tiempo de espera para una trama incompleta
const unsigned long timeout = 100;

PS2X ps2x;
QGPMaker_MotorShield AFMS = QGPMaker_MotorShield();

QGPMaker_DCMotor *DCMotor_1 = AFMS.getMotor(1);
QGPMaker_DCMotor *DCMotor_2 = AFMS.getMotor(2);
QGPMaker_DCMotor *DCMotor_3 = AFMS.getMotor(3);
QGPMaker_DCMotor *DCMotor_4 = AFMS.getMotor(4);

QGPMaker_Encoder encoder1 = QGPMaker_Encoder(2);
QGPMaker_Encoder encoder2 = QGPMaker_Encoder(3);
QGPMaker_Encoder encoder3 = QGPMaker_Encoder(1);
QGPMaker_Encoder encoder4 = QGPMaker_Encoder(0);

int pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
unsigned long lastTime = 0;
int speedMotor1 = 0, speedMotor2 = 0, speedMotor3 = 0, speedMotor4 = 0;

void setup(){
  Serial.begin(9600); // Cambiamos la velocidad para que coincida con el código de PC
  Serial.println("Esperando trama HDLC...");
  AFMS.begin(50);

  // Configuración del PS2
  int error = 0;
  do {
    error = ps2x.config_gamepad(13, 11, 10, 12, true, true);
    if (error == 0) {
      break;
    } else {
      delay(100);
    }
  } while (1);
}

void loop() {
  if (Serial.available() > 0) {
    uint8_t byte = Serial.read();
    last_byte_time = millis();
    process_byte(byte);
  }

  // Verificar si ha pasado el tiempo de espera sin recibir más bytes
  if (rx_length > 0 && millis() - last_byte_time > timeout) {
    //Serial.println("Error: Trama incompleta descartada.");
    rx_length = 0;  // Restablecer la longitud de la trama
    rx_crc = HDLC_CRC_INIT_VALUE;
  }


  ps2x.read_gamepad(false, 0);
  delay(30);
  checkPS2Buttons();
  delay(2);
}

// Función para actualizar el CRC
uint16_t crc_update(uint16_t crc, uint8_t data) {
    data ^= (uint8_t)(crc & 0xFF);
    data ^= (data << 4);
    return ((((uint16_t)data << 8) | ((uint8_t)(crc >> 8) & 0xFF)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}

// Función para procesar cada byte recibido
void process_byte(uint8_t byte) {
  if (byte == HDLC_FRAME_BOUNDRY_FLAG) {
    if (rx_length > 0) { // Procesamos la trama sin verificar el CRC para pruebas
      //Serial.println("Trama recibida");
      process_frame(rx_buffer, rx_length - 2);
    }
    rx_length = 0;
    rx_crc = HDLC_CRC_INIT_VALUE;
    return;
  }

  if (byte == HDLC_ESCAPE_FLAG) {
    escape_next = true;
    return;
  }
  if (escape_next) {
    byte ^= HDLC_ESCAPE_XOR;
    escape_next = false;
  }

  if (rx_length < sizeof(rx_buffer)) {
    rx_buffer[rx_length++] = byte;
    if (rx_length >= 3) {
      rx_crc = crc_update(rx_crc, rx_buffer[rx_length - 3]);
    }
  }
}

// Procesa el frame recibido y ejecuta el comando
void process_frame(uint8_t *data, int length) {
    String command = String((char*)data).substring(0, length);
    //Serial.print("Comando recibido: ");
    //Serial.println(command);
    processCommand(command);
}

// Función para procesar comandos recibidos
void processCommand(String command) {
    if (!command.startsWith("M:")) {
        return;
    }

    int speed1 = command.substring(2, command.indexOf(':', 2)).toInt();
    int speed2 = command.substring(command.indexOf(':', 2) + 1, command.indexOf(':', command.indexOf(':', 2) + 1)).toInt();
    int speed3 = command.substring(command.indexOf(':', command.indexOf(':', 2) + 1) + 1, command.lastIndexOf(':')).toInt();
    int speed4 = command.substring(command.lastIndexOf(':') + 1).toInt();

    speedMotor1 = speed1;
    speedMotor2 = speed2;
    speedMotor3 = speed3;
    speedMotor4 = speed4;

    setMotorSpeed(DCMotor_1, speed1);
    setMotorSpeed(DCMotor_2, speed2);
    setMotorSpeed(DCMotor_3, speed3);
    setMotorSpeed(DCMotor_4, speed4);

    long newPos1 = encoder1.read();
    long newPos2 = encoder2.read();
    long newPos3 = encoder3.read();
    long newPos4 = encoder4.read();

    unsigned long currentTime = millis();
    unsigned long deltaTime = currentTime - lastTime;
  
    pos1 = newPos1;
    pos2 = newPos2;
    pos3 = newPos3;
    pos4 = newPos4;

    lastTime = currentTime;

    sendMotorData();
}

// Función para establecer la velocidad de cada motor
void setMotorSpeed(QGPMaker_DCMotor *motor , int speed) {
    motor->setSpeed(abs(speed));
    motor->run(speed >= 0 ? FORWARD : BACKWARD);
}

// Enviar información de posición y velocidad HDLC
void sendMotorData() {
    String data = "V:" + String(speedMotor1) + ":" + String(speedMotor2) + ":" + String(speedMotor3) + ":" + String(speedMotor4);
    //Serial.println("SPEED:" + data);
    //Serial.println(data);
    Serial.println();
    send_frame((const uint8_t*)data.c_str(), data.length());
}

// Enviar una trama HDLC
void send_frame(const uint8_t* data, size_t size) {
    uint8_t frame[512];
    size_t frame_length = 0;
    uint16_t crc = HDLC_CRC_INIT_VALUE;

    frame[frame_length++] = HDLC_FRAME_BOUNDRY_FLAG;
    for (size_t i = 0; i < size; i++) {
        crc = crc_update(crc, data[i]);
        if (data[i] == HDLC_ESCAPE_FLAG || data[i] == HDLC_FRAME_BOUNDRY_FLAG) {
            frame[frame_length++] = HDLC_ESCAPE_FLAG;
            frame[frame_length++] = data[i] ^ HDLC_ESCAPE_XOR;
        } else {
            frame[frame_length++] = data[i];
        }
    }

    // Añadir el CRC al final del frame, también aplicando escape si es necesario
    uint8_t crc_low = crc & 0xFF;
    uint8_t crc_high = (crc >> 8) & 0xFF;
    if (crc_low == HDLC_ESCAPE_FLAG || crc_low == HDLC_FRAME_BOUNDRY_FLAG) {
        frame[frame_length++] = HDLC_ESCAPE_FLAG;
        frame[frame_length++] = crc_low ^ HDLC_ESCAPE_XOR;
    } else {
        frame[frame_length++] = crc_low;
    }
    if (crc_high == HDLC_ESCAPE_FLAG || crc_high == HDLC_FRAME_BOUNDRY_FLAG) {
        frame[frame_length++] = HDLC_ESCAPE_FLAG;
        frame[frame_length++] = crc_high ^ HDLC_ESCAPE_XOR;
    } else {
        frame[frame_length++] = crc_high;
    }

    // Añadir delimitador de fin
    frame[frame_length++] = HDLC_FRAME_BOUNDRY_FLAG;

    // Verificación: Imprimir el frame en hexadecimal en el monitor serial
    //Serial.print("Frame enviado: ");
    for (size_t i = 0; i < frame_length; i++) {
        //Serial.print(frame[i], HEX);
        //Serial.print(" ");
    }
    //Serial.println();

    // Enviar el frame completo por el puerto serie
    Serial.write(frame, frame_length);
}

// Verificar botones del PS2
void checkPS2Buttons() {
  if (ps2x.Button(PSB_PAD_UP)) {
    forward();
  } else if(ps2x.Button(PSB_PAD_DOWN)){
    backward();
  } else if (ps2x.Button(PSB_PAD_LEFT)) {
    turnLeft();
  } else if (ps2x.Button(PSB_PAD_RIGHT)) {
    turnRight();
  } else if (ps2x.Button(PSB_L1)) {
    moveLeft();
  } else if (ps2x.Button(PSB_R1)) {
    moveRight();
  } else if (ps2x.Button(PSB_CIRCLE)){
    stopMoving();
  }
}

// Funciones de movimiento de los motores
void forward() {
  String command = "M:200:200:200:200";
  processCommand(command);
}

void turnLeft() {
  String command = "M:-200:-200:200:200";
  processCommand(command);
}
void turnRight() {
  String command = "M:200:200:-200:-200";
  processCommand(command);
}
void moveLeft() {
  String command = "M:-200:200:-200:200";
  processCommand(command);
}

void moveRight() {
  String command = "M:200:-200:200:-200";
  processCommand(command);
}

void backward() {
  String command = "M:-200:-200:-200:-200";
  processCommand(command);
}

void stopMoving() {
  String command = "M:0:0:0:0";
  processCommand(command);
}
