import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    package_share = get_package_share_directory("pcd_map_publisher")
    config_file = os.path.join(package_share, "config", "pcd_map_publisher.yaml")

    return LaunchDescription([
        Node(
            package="pcd_map_publisher",
            executable="pcd_map_publisher_node",
            name="pcd_map_publisher",
            output="screen",
            parameters=[config_file],
        )
    ])
