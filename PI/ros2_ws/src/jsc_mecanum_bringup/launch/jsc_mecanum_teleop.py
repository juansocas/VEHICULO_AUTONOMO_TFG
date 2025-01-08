import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    teleop_config = os.path.join(get_package_share_directory('jsc_mecanum_teleop'), 'config', 'teleop_config.yaml')

    return LaunchDescription([
        Node(
            package='jsc_mecanum_teleop',
            executable='jsc_mecanum_teleop',
            output='screen',
            parameters=[
                teleop_config
            ]
        )
    ])