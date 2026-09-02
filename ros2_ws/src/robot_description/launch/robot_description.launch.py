import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory('robot_description')
    
    # Target the actual URDF file directly
    urdf_file = os.path.join(pkg_share, 'urdf', 'assembly_1.urdf')

    # Read the URDF file directly
    with open(urdf_file, 'r') as f:
        robot_desc = f.read()

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_desc}]
    )

    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        output='screen',
        parameters=[{
            'zeros.parallel_1': 0.1,
            'zeros.parallel_2_1': -0.053,
            'zeros.parallel_3_1': -0.02,
            'zeros.parallel_3_loop_closure_1': 0.053,
        }]
    )

    static_tf_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='map_to_robot_pose',
        arguments=['0', '0', '0', '0', '0', '0', 'odom', 'robot_pose']
    )

    static_tf_node1 = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='robot_pose_to_root',
        arguments=['0', '0', '0', '0', '0', '0', 'robot_pose', 'root']
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen'
    )

    return LaunchDescription([
        robot_state_publisher_node,
        joint_state_publisher_gui_node,
        static_tf_node,
        static_tf_node1,
        rviz_node
    ])