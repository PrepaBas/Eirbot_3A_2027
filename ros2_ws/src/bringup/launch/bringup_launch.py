# example.launch.py

import os

from ament_index_python import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.actions import GroupAction
from launch.launch_description_sources import AnyLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch.substitutions import TextSubstitution
from launch_ros.actions import Node
from launch_ros.actions import PushRosNamespace


def generate_launch_description():
    # include another launch file
    launch_robot_visualization = IncludeLaunchDescription(
        AnyLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('robot_visualization'),
                'launch/robot_visualization_launch.xml',
            )
        )
    )

    # start a turtlesim_node in the turtlesim1 namespace
    rviz_node = Node(
            package='rviz2',
            executable='rviz2',
            name='robot_viz',
            arguments=['-d' + os.path.join(get_package_share_directory('robot_visualization'), 'rviz', 'config.rviz')]
        )

    return LaunchDescription([
      launch_robot_visualization,
        rviz_node,
    ])