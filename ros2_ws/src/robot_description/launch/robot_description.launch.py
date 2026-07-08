import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node

def generate_launch_description():
    # Get the path to the robot_description package
    urdf_file_name = 'assembly_1.launch'
    robot_description_path = get_package_share_directory('robot_description')

    # Define the path to the URDF file
    urdf_file = os.path.join(robot_description_path, 'launch', urdf_file_name)

    # Create a Node to publish the robot description
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': Command(['xacro ', urdf_file])
        }]
    )

    # Create a Node to launch RViz
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        #arguments=['-d', os.path.join(robot_description_path, 'rviz', 'robot_description.rviz')]
    )

    return LaunchDescription([
        robot_state_publisher_node,
        rviz_node
    ])