import serial

ser = serial.Serial('/dev/ttyUSB0', 9600)  # Asegúrate de que sea el puerto correcto

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        print(f"Datos recibidos: {line}")
