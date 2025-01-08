
#! Author: Juan Socas
#! Description: Launch a JSC mobile robot with 4 wheels

import os

from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import Command, LaunchConfiguration
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import IncludeLaunchDescription


def generate_launch_description():

    pkg_share = FindPackageShare(package='jsc_mecanum_description').find('jsc_mecanum_description')
    default_launch_dir = os.path.join(pkg_share, 'launch')
    default_model_path = os.path.join(pkg_share, 'src/jsc_mecanum_description.urdf')
    robot_name_in_urdf = 'jsc_mecanum_bot'
    default_rviz_config_path = os.path.join(pkg_share, 'rviz/model.rviz')
    

    # LIDAR configuration
    channel_type =  LaunchConfiguration('channel_type', default='serial')
    serial_port = LaunchConfiguration('serial_port', default='/dev/ttyUSB0')
    serial_baudrate = LaunchConfiguration('serial_baudrate', default='1000000') #for s2 is 1000000
    frame_id = LaunchConfiguration('frame_id', default='lidar_link')
    inverted = LaunchConfiguration('inverted', default='false')
    angle_compensate = LaunchConfiguration('angle_compensate', default='true')
    scan_mode = LaunchConfiguration('scan_mode', default='DenseBoost')





    # Launch configuration variables specific to simulation
    gui = LaunchConfiguration('gui')
    model = LaunchConfiguration('model')
    rviz_config_file = LaunchConfiguration('rviz_config_file')
    use_robot_state_pub = LaunchConfiguration('use_robot_state_pub')
    use_rviz = LaunchConfiguration('use_rviz')
    use_sim_time = LaunchConfiguration('use_sim_time')


    # Declare the launch arguments  
    declare_model_path_cmd = DeclareLaunchArgument(
        name='model', 
        default_value=default_model_path, 
        description='Absolute path to robot urdf file')
        
    declare_rviz_config_file_cmd = DeclareLaunchArgument(
        name='rviz_config_file',
        default_value=default_rviz_config_path,
        description='Full path to the RVIZ config file to use')
        
    declare_use_joint_state_publisher_cmd = DeclareLaunchArgument(
        name='gui',
        default_value='True',
        description='Flag to enable joint_state_publisher_gui')
    
    declare_use_robot_state_pub_cmd = DeclareLaunchArgument(
        name='use_robot_state_pub',
        default_value='True',
        description='Whether to start the robot state publisher')

    declare_use_rviz_cmd = DeclareLaunchArgument(
        name='use_rviz',
        default_value='True',
        description='Whether to start RVIZ')
        
    declare_use_sim_time_cmd = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='False',
        description='Use simulation (Gazebo) clock if true')
    

    #LIDAR

    declare_channet_type = DeclareLaunchArgument(
        name ='channel_type',
        default_value=channel_type,
        description='Specifying channel type of lidar')
    
    declare_serial_port = DeclareLaunchArgument(
        name='serial_port',
        default_value=serial_port,
        description='Specifying usb port to connected lidar')
    
    declare_serial_baudrate = DeclareLaunchArgument(
        name='serial_baudrate',
        default_value=serial_baudrate,
        description='Specifying usb port baudrate to connected lidar')


    declare_frame_id = DeclareLaunchArgument(
        name='frame_id',
        default_value=frame_id,
        description='Specifying frame_id of lidar')

    declare_invert_scan = DeclareLaunchArgument(
        name='inverted',
        default_value=inverted,
        description='Specifying whether or not to invert scan data')


    declare_angle_compensate = DeclareLaunchArgument(
        name='angle_compensate',
        default_value=angle_compensate,
        description='Specifying whether or not to enable angle_compensate of scan data')

    declare_scan_mode = DeclareLaunchArgument(
        name='scan_mode',
        default_value=scan_mode,
        description='Specifying scan mode of lidar')
    

    
    # Specify the actions

    # Publish the joint state values for the non-fixed joints in the URDF file.
    start_joint_state_publisher_cmd = Node(
        condition=UnlessCondition(gui),
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher')

    # A GUI to manipulate the joint state values
    start_joint_state_publisher_gui_node = Node(
        condition=IfCondition(gui),
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui')

    # Subscribe to the joint states of the robot, and publish the 3D pose of each link.
    start_robot_state_publisher_cmd = Node(
        condition=IfCondition(use_robot_state_pub),
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'use_sim_time': use_sim_time, 
        'robot_description': Command(['xacro ', model])}],
        arguments=[default_model_path])

    # Launch RViz
    start_rviz_cmd = Node(
        condition=IfCondition(use_rviz),
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file])
    
    # Launch LIDAR

    start_lidar_cmd = Node(
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
        output='screen')
    
    start_lidar_transform_cmd = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='lidar_to_base_link',
        output='screen',
        arguments=['0.05', '0', '0.085', '0', '0', '0', 'base_link', 'lidar_link']
        )

    # Transformación de base_footprint a base_link
    tf_basefootprint_to_baselink = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='basefootprint_to_baselink',
        output='screen',
        arguments=['0', '0', '0', '0', '0', '0', 'base_footprint', 'base_link']
    )


    # Transformación de base_link a map
    tf_baselink_to_map = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='baselink_to_map',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'base_link']
    )


    slam_cmd = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[os.path.join(pkg_share, 'config', 'slam_toolbox_params.yaml')],
        remappings=[
            ('/scan', '/scan')
        ]
    )

    tf_slam = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=["0", "0", "0", "0", "0", "0", "odom", "base_footprint"]
    )

    TF_cmd =Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_to_map',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
        arguments=['0', '0', '0', '0', '0', '0', 'map', 'base_footprint'],  
    )





    # Create the launch description and populate
    ld = LaunchDescription()

     # Declare the launch options
    ld.add_action(declare_model_path_cmd)
    ld.add_action(declare_rviz_config_file_cmd)
    ld.add_action(declare_use_joint_state_publisher_cmd)
    ld.add_action(declare_use_robot_state_pub_cmd)  
    ld.add_action(declare_use_rviz_cmd) 
    ld.add_action(declare_use_sim_time_cmd)

    ld.add_action(declare_channet_type)
    ld.add_action(declare_serial_port)
    ld.add_action(declare_serial_baudrate)
    ld.add_action(declare_frame_id)
    ld.add_action(declare_invert_scan)
    ld.add_action(declare_angle_compensate)
    ld.add_action(declare_scan_mode)
    #ld.add_action(declare_controller_manager)
    

    # Add any actions
    ld.add_action(start_joint_state_publisher_cmd)
    ld.add_action(tf_basefootprint_to_baselink)
    ld.add_action(tf_baselink_to_map)

    # GUI of wheel rotation
    #ld.add_action(start_joint_state_publisher_gui_node)
    ld.add_action(start_robot_state_publisher_cmd)
    ld.add_action(start_lidar_cmd)
    ld.add_action(start_lidar_transform_cmd)
  

    #! Trasform Tree for LIDAR to base
    #ld.add_action(tf_slam)
    #ld.add_action(TF_cmd)

    #ld.add_action(slam_cmd)
    #ld.add_action(Lidar_ondo)
    #ld.add_action(cartographer_launch_cmd)
    #ld.add_action(occupancy_grid_launch_cmd)

    return ld