import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import serial

class SerialMovementNode(Node):

    def __init__(self):
        super().__init__('serial_movement_node')
        #! JSC: Creamos un subscriptor al topic => serial_movimiento_entrada
        self.subscription = self.create_subscription(
            String,
            'serial_movimiento_entrada',
            self.callback,
            10
        )
        #! JSC: Creamos un subscriptor al topic => serial_movimiento_salida
        self.publisher = self.create_publisher(
            String,
            'serial_movimiento_salida',
            10
        )
        #! JSC: Abre el puerto serie
        self.serial_port = serial.Serial('/dev/ttyUSB0', 9600)  

    def callback(self, msg):
        #! JSC: Ver lo que se recibe del topic => serial_movimiento_entrada
        self.get_logger().info(f'Received: {msg.data}')
        #! JSC: Procesamiento de la información recibida
        processed_msg = String()
        processed_msg.data = msg.data[::]
        #! JSC: Mandamos esa informacion al nuevo topic => serial_movimiento_salida
        self.publisher.publish(processed_msg)
        self.get_logger().info(f'Published: {processed_msg.data}')
        
        if self.serial_port.is_open:
            #! JSC: Enviamos el mensaje recibido de vuelta al puerto serie
            self.serial_port.write(processed_msg.data.encode())
            self.get_logger().info(f'Sent data back to serial port: {processed_msg.data}')


def main(args=None):
    rclpy.init(args=args)
    serial_movement_node = SerialMovementNode()
    rclpy.spin(serial_movement_node)
    serial_movement_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
