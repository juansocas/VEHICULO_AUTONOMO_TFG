from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='motor_gui_pkg',
            executable='motor_gui',
            name='motor_gui',
            output='screen',
        ),
    ])
