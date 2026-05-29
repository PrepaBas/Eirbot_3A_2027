#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point
import os

class ImageMarkerPublisher(Node):
    def __init__(self):
        super().__init__('image_marker_publisher')
        self.publisher_ = self.create_publisher(Marker, 'image_marker', 10)
        self.timer = self.create_timer(1.0, self.timer_callback)
        
        # Absolute path to your PNG image
        # NOTE: RViz standard markers don't natively load raw local PNG binaries directly into textures 
        # via the standard Mesh resource without a URI. 
        # A bulletproof approach for a flat ground image is utilizing a MESH_RESOURCE marker pointing to your file.
        self.image_path = "file:///home/ros/ros2_ws/src/rviz_config/my_image.png" 

    def timer_callback(self):
        marker = Marker()
        marker.header.frame_id = "map"
        marker.header.stamp = self.get_clock().now().to_msg()
        marker.ns = "ground_plane"
        marker.id = 0
        
        # Use a MESH_RESOURCE type to display an external file/plane
        marker.type = Marker.MESH_RESOURCE
        marker.mesh_resource = self.image_path
        marker.mesh_use_embedded_materials = True # Keeps original PNG colors
        
        marker.action = Marker.ADD

        # Position: Centered at x=0, y=0, and flat on the ground z=0
        marker.pose.position.x = 0.0
        marker.pose.position.y = 0.0
        marker.pose.position.z = 0.0
        
        # Orientation: Perfectly flat (No rotation)
        marker.pose.orientation.x = 0.0
        marker.pose.orientation.y = 0.0
        marker.pose.orientation.z = 0.0
        marker.pose.orientation.w = 1.0

        # Scale: Adjust these numbers to match the physical size (in meters) 
        # you want the image to be in your simulation environment!
        marker.scale.x = 5.0  # Width in meters
        marker.scale.y = 5.0  # Height in meters
        marker.scale.z = 0.01 # Make it paper-thin

        # Color alpha (1.0 = fully opaque, 0.5 = semi-transparent so you can see grid lines)
        marker.color.a = 1.0 

        self.publisher_.publish(marker)
        self.get_logger().info('Publishing ground image marker...')

def main(args=None):
    rclpy.init(args=args)
    node = ImageMarkerPublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
