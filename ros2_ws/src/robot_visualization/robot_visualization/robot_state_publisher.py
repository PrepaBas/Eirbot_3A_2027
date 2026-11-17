import math
import rclpy
from rclpy.node import Node
from tf2_ros import TransformBroadcaster, TransformStamped
from sensor_msgs.msg import JointState

def quaternion_from_euler(roll, pitch, yaw):
    """
    Converts euler roll, pitch, yaw to quaternion (w in last place)
    quat = [x, y, z, w]
    Bellow should be replaced when porting for ROS 2 Python tf_conversions is done.
    """
    cy = math.cos(yaw * 0.5)
    sy = math.sin(yaw * 0.5)
    cp = math.cos(pitch * 0.5)
    sp = math.sin(pitch * 0.5)
    cr = math.cos(roll * 0.5)
    sr = math.sin(roll * 0.5)

    q = [0] * 4
    q[0] = cy * cp * cr + sy * sp * sr
    q[1] = cy * cp * sr - sy * sp * cr
    q[2] = sy * cp * sr + cy * sp * cr
    q[3] = sy * cp * cr - cy * sp * sr

    return q

class RobotStatePublisher(Node):

    def __init__(self):
        super().__init__("robot_state_publisher")

        # Create publisher of message type 'JointState' onto the /joint_states topic
        self.joint_pub = self.create_publisher(JointState, 'joint_states', 10)
        self.joint_state = JointState()   

        # Create a publisher for tree transforms. This publishes pose in /odom.
        # Use of TransformBroadcaster because of tf2 complexity
        self.odom_broadcaster = TransformBroadcaster(self, 10)
        self.odom_tf = TransformStamped()
        self.odom_tf.header.frame_id = 'map'
        self.odom_tf.child_frame_id = 'odom'

        # Create a timer of frequency 100Hz (T = 0.01 s)
        self.timer = self.create_timer(0.01, self.timer_callback)
        self.i = 0

    def timer_callback(self):
        now = self.get_clock().now()

        # Joint Publisher                       # creation
        self.joint_state.header.stamp = now.to_msg()             # time in header
        self.joint_state.name = ['left_wheel', 'right_wheel']  # list of joints
        self.joint_state.position = [2, 5]                     # value of joints

        # Odom transform Publisher
        self.odom_tf.header.stamp = now.to_msg()
        self.odom_tf.transform.translation.x = math.cos(0.01 * self.i)
        self.odom_tf.transform.translation.y = math.sin(0.01 * self.i)
        q = quaternion_from_euler(0, 0, - 0.01 * self.i)
        self.odom_tf.transform.rotation.x = q[1]
        self.odom_tf.transform.rotation.y = q[2]
        self.odom_tf.transform.rotation.z = q[3]
        self.odom_tf.transform.rotation.w = q[0]


        # Wrap up
        self.joint_pub.publish(self.joint_state)
        self.odom_broadcaster.sendTransform(self.odom_tf)
        # self.get_logger().info('Publishing joint state "%i"', self.i)
        self.i +=1


def main():
    print('Hi from robot_visualization.')
    rclpy.init()
    robot_state_publisher = RobotStatePublisher()
    rclpy.spin(robot_state_publisher)

    robot_state_publisher.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
