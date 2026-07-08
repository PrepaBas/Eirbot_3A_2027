
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument 
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

def generate_launch_description(): 
    bringup_pkg = get_package_share_directory('robot_bringup')


    ## Declare the launch argument for the master parameter file
    declare_master_param = DeclareLaunchArgument( ## add this to the return list
        'master_param_file', 
        default_value='empty.yaml',
        description='Yaml file containing overriding parameters for all nodes'
    )
    
    master_param_config = LaunchConfiguration('master_param_file')

    master_param = PathJoinSubstitution([
        bringup_pkg,
        'config',
        master_param_config
    ])
    


    return LaunchDescription([
        declare_master_param,


        Node(
            package = 'tf2_ros',
            executable = 'static_transform_publisher',
            name = 'map_to_odom',
            arguments = ["0", "0", "0", "0", "0", "0", "odom", "laser"]
        ),
        Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[os.path.join(bringup_pkg, 'config', 'ekf.yaml'), master_param]
        )
    ])