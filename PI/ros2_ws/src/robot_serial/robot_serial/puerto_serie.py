import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import serial

class SerialNode(Node):

    def __init__(self):
        super().__init__('serial_node')
        self.publisher_ = self.create_publisher(String, 'serial_movimiento_entrada', 10)
        self.serial_port = serial.Serial('/dev/ttyUSB0', 9600)  # Configura el puerto serie

        # !Crea un temporizador para leer datos del puerto serie y publicarlos
        self.timer = self.create_timer(0.1, self.publish_serial_data)

    def publish_serial_data(self):
        if self.serial_port.in_waiting > 0:
            serial_data = self.serial_port.readline().decode('utf-8').strip()
            msg = String()
            msg.data = serial_data
            self.publisher_.publish(msg)
            self.get_logger().info(f'Published to serial_movimiento_entrada: {serial_data}')
    

def main(args=None):
    rclpy.init(args=args)
    serial_node = SerialNode()
    rclpy.spin(serial_node)
    serial_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
