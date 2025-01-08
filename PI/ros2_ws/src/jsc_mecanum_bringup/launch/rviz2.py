# Author: Juan Socas
# Description: Launch a JSC mobile robot with 4 wheels

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import Command, LaunchConfiguration

def generate_launch_description():
    rviz_config_dir = os.path.join(
        get_package_share_directory('jsc_mecanum_description'),
        'rviz',
        'model.rviz')
    


    # LIDAR
    channel_type =  LaunchConfiguration('channel_type', default='serial')
    serial_port = LaunchConfiguration('serial_port', default='/dev/ttyUSB1')
    serial_baudrate = LaunchConfiguration('serial_baudrate', default='1000000') #for s2 is 1000000
    frame_id = LaunchConfiguration('frame_id', default='lidar_link')
    inverted = LaunchConfiguration('inverted', default='false')
    angle_compensate = LaunchConfiguration('angle_compensate', default='true')
    scan_mode = LaunchConfiguration('scan_mode', default='DenseBoost')
    

    

    return LaunchDescription([

        DeclareLaunchArgument(
            name ='channel_type',
            default_value=channel_type,
            description='Specifying channel type of lidar'),

        DeclareLaunchArgument(
            name='serial_port',
            default_value=serial_port,
            description='Specifying usb port to connected lidar'),

        DeclareLaunchArgument(
            name='serial_baudrate',
            default_value=serial_baudrate,
            description='Specifying usb port baudrate to connected lidar'),

        DeclareLaunchArgument(
            name='frame_id',
            default_value=frame_id,
            description='Specifying frame_id of lidar'),
        
        DeclareLaunchArgument(
            name='inverted',
            default_value=inverted,
            description='Specifying whether or not to invert scan data'),

        DeclareLaunchArgument(
            name='angle_compensate',
            default_value=angle_compensate,
            description='Specifying whether or not to enable angle_compensate of scan data'),

        DeclareLaunchArgument(
            name='scan_mode',
            default_value=scan_mode,
            description='Specifying scan mode of lidar'),


        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            arguments=['-d', rviz_config_dir]),

        Node(
            package='sllidar_ros2',
            executable='sllidar_node',
            name='sllidar_node',
            parameters=[{'channel_type':channel_type,
                        'serial_port': serial_port, 
                        'serial_baudrate': serial_baudrate, 
                        'frame_id': frame_id,
                        'inverted': inverted, 
                        'angle_compensate': angle_compensate,
                        'scan_mode': scan_mode
                            }],
            output='screen',
            arguments=['0.05', '0', '0.085', '0', '0', '0', 'base_link', 'lidar_link']),
       

    ])