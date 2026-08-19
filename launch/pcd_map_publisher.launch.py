import os

import yaml

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def _load_config(config_file):
    if not os.path.isfile(config_file):
        raise RuntimeError(f"Config file '{config_file}' does not exist.")

    with open(config_file, "r", encoding="utf-8") as handle:
        config = yaml.safe_load(handle) or {}

    if not isinstance(config, dict):
        raise RuntimeError(
            f"Config file '{config_file}' must contain a YAML mapping at the top level."
        )

    return config


def _normalize_publishers(config, config_file):
    publishers = config.get("publishers")
    if publishers is not None:
        defaults = config.get("defaults", {})
        if not isinstance(defaults, dict):
            raise RuntimeError(
                f"Config file '{config_file}' has an invalid 'defaults' section."
            )
        if not isinstance(publishers, list) or not publishers:
            raise RuntimeError(
                f"Config file '{config_file}' must define a non-empty 'publishers' list."
            )

        normalized_publishers = []
        node_names = set()
        for index, publisher in enumerate(publishers, start=1):
            if not isinstance(publisher, dict):
                raise RuntimeError(
                    f"Publisher entry #{index} in '{config_file}' must be a YAML mapping."
                )

            parameters = defaults.copy()
            parameters.update(publisher)
            node_name = parameters.pop("name", f"pcd_map_publisher_{index}")

            if not isinstance(node_name, str) or not node_name:
                raise RuntimeError(
                    f"Publisher entry #{index} in '{config_file}' has an invalid 'name'."
                )
            if node_name in node_names:
                raise RuntimeError(
                    f"Config file '{config_file}' contains duplicate publisher name '{node_name}'."
                )
            if not parameters.get("pcd_file_path"):
                raise RuntimeError(
                    f"Publisher '{node_name}' in '{config_file}' is missing 'pcd_file_path'."
                )
            if not parameters.get("topic_name"):
                raise RuntimeError(
                    f"Publisher '{node_name}' in '{config_file}' is missing 'topic_name'."
                )

            node_names.add(node_name)
            normalized_publishers.append((node_name, parameters))

        return normalized_publishers

    legacy_node = config.get("pcd_map_publisher", {})
    if not isinstance(legacy_node, dict):
        raise RuntimeError(
            f"Config file '{config_file}' must define either 'publishers' or 'pcd_map_publisher'."
        )

    legacy_parameters = legacy_node.get("ros__parameters")
    if isinstance(legacy_parameters, dict) and legacy_parameters:
        return [("pcd_map_publisher", legacy_parameters.copy())]

    raise RuntimeError(
        f"Config file '{config_file}' must define either a 'publishers' list or "
        "'pcd_map_publisher.ros__parameters'."
    )


def _create_publisher_nodes(context):
    config_file = LaunchConfiguration("publishers_config").perform(context)
    config = _load_config(config_file)
    publishers = _normalize_publishers(config, config_file)

    return [
        Node(
            package="pcd_map_publisher",
            executable="pcd_map_publisher_node",
            name=node_name,
            output="screen",
            parameters=[parameters],
        )
        for node_name, parameters in publishers
    ]


def generate_launch_description():
    package_share = get_package_share_directory("pcd_map_publisher")
    default_config_file = os.path.join(package_share, "config", "pcd_map_publishers.yaml")

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "publishers_config",
                default_value=default_config_file,
                description="YAML file describing one or more PCD publishers.",
            ),
            OpaqueFunction(function=_create_publisher_nodes),
        ]
    )
