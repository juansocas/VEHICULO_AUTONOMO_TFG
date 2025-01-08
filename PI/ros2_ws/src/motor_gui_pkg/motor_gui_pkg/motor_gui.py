import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from PyQt5 import QtWidgets, QtCore

class MotorControlGUI(Node):
    def __init__(self):
        super().__init__('motor_control_gui')
        
        # Configura el publicador para enviar mensajes TwistStamped en el tópico de comando de velocidad
        self.publisher = self.create_publisher(Twist, '/cmd_vel', 10)
        
        # Configura la interfaz gráfica
        self.app = QtWidgets.QApplication([])
        self.window = QtWidgets.QWidget()
        self.window.setWindowTitle("Control de Velocidad de Robot")
        self.layout = QtWidgets.QVBoxLayout()

        # Crear sliders y conectar eventos para el movimiento general

        #! Control para movimiento hacia adelante/atrás
        self.forward_slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.forward_slider.setMinimum(-4)  # Control para movimiento hacia adelante/atrás
        self.forward_slider.setMaximum(4)
        self.forward_slider.setValue(0)
        self.forward_slider.valueChanged.connect(self.publish_speeds)
        self.layout.addWidget(QtWidgets.QLabel("Movimiento Atrás/Adelante"))
        self.layout.addWidget(self.forward_slider)

        #! Control para movimiento lateral
        self.side_slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.side_slider.setMinimum(-4)  # Control para movimiento lateral
        self.side_slider.setMaximum(4)
        self.side_slider.setValue(0)
        self.side_slider.valueChanged.connect(self.publish_speeds)
        self.layout.addWidget(QtWidgets.QLabel("Movimiento Lateral Der/Izq"))
        self.layout.addWidget(self.side_slider)
        #! Control para rotación
        self.rotation_slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.rotation_slider.setMinimum(-4)  # Control para rotación
        self.rotation_slider.setMaximum(4)
        self.rotation_slider.setValue(0)
        self.rotation_slider.valueChanged.connect(self.publish_speeds)
        self.layout.addWidget(QtWidgets.QLabel("Rotación Der/Izq"))
        self.layout.addWidget(self.rotation_slider)

        # Agrega un botón para detener el movimiento
        self.stop_button = QtWidgets.QPushButton("Detener Robot")
        self.stop_button.clicked.connect(self.stop_robot)
        self.layout.addWidget(self.stop_button)

        # Configura la ventana y el layout
        self.window.setLayout(self.layout)
        self.window.show()

    def publish_speeds(self):
        twist = Twist()
        twist.linear.x = float(self.forward_slider.value())
        
        twist.linear.y = float(self.side_slider.value())
        twist.angular.z = float(self.rotation_slider.value())
        self.publisher.publish(twist)

        # Log para ver los valores enviados
        self.get_logger().info(f"Velocidades enviadas - Adelante/Atrás: {self.forward_slider.value()}, "
                               f"Lateral Izq/Der: {self.side_slider.value()}, Rotación: {self.rotation_slider.value()}")

    def stop_robot(self):
        # Detener el robot y resetear sliders
        self.forward_slider.setValue(0)
        self.side_slider.setValue(0)
        self.rotation_slider.setValue(0)
        self.publish_speeds()

    def run(self):
        # Ejecutar la aplicación de GUI
        self.app.exec_()

def main(args=None):
    rclpy.init(args=args)
    gui = MotorControlGUI()
    gui.run()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
