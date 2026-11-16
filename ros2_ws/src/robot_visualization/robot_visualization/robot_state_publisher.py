import rclpy
from rclpy.node import Node
from tf2_ros import TransformBroadcaster, TransformStamped

from sensor_msgs.msg import JointState

class RobotStatePublisher(Node):

    def __init__(self):
        super().__init__("robot_state_publisher")

        # Create publisher of message type 'JointState' onto the /joint_states topic
        self.publisher_ = self.create_publisher(JointState, 'joint_states', 10)
        
        # Create a timer of frequency 10Hz (T = 0.1 s)
        self.timer = self.create_timer(0.1, self.timer_callback)
        self.i = 0

    def timer_callback(self):
        msg = JointState()
        now = self.get_clock().now()
        msg.name = ['left_wheel', 'right_wheel']
        msg.position = [2, 5]
        self.publisher_.publish(msg)
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
