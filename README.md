# pcd_map_publisher

Simple ROS 2 package that loads a `.pcd` file and publishes it as
`sensor_msgs/msg/PointCloud2`. The launch file can start one or more publisher
instances, so you can publish `n >= 1` PCD files to different topics without
changing the C++ node.

## Multi-Publisher Launch Config

Edit `config/pcd_map_publishers.yaml`:

```yaml
defaults:
  frame_id: "map"
  publish_rate_hz: 1.0
  use_transient_local: false
  enable_voxel_downsampling: true
  voxel_leaf_size: 0.2

publishers:
  - name: "pcd_map_publisher_356"
    pcd_file_path: "/path/to/356.pcd"
    topic_name: "/pcd_map_356"

  - name: "pcd_map_publisher_512"
    pcd_file_path: "/path/to/512.pcd"
    topic_name: "/pcd_map_512"
    publish_rate_hz: 0.5
```

Each item in `publishers` starts one `pcd_map_publisher_node` instance.

- `name`: ROS node name for that publisher instance
- `pcd_file_path`: absolute path to the PCD file to load
- `topic_name`: topic used for publishing the point cloud
- `frame_id`: frame id stored in the message header
- `publish_rate_hz`: periodic publish rate
- `use_transient_local`: enable transient local QoS for late subscribers
- `enable_voxel_downsampling`: whether to downsample the map before publishing
- `voxel_leaf_size`: voxel size in meters used for downsampling

`defaults` is optional and applies to every publisher unless overridden by a
specific entry.

## Legacy Single-Publisher Config

The launch file still accepts the old single-node parameter file format in
`config/pcd_map_publisher.yaml`:

```yaml
pcd_map_publisher:
  ros__parameters:
    pcd_file_path: "/path/to/map.pcd"
    topic_name: "/pcd_map"
    frame_id: "map"
```

## Build

```bash
cd /home/rizy/ws_tools
colcon build --packages-select pcd_map_publisher
```

## Run

```bash
source install/setup.bash
ros2 launch pcd_map_publisher pcd_map_publisher.launch.py
```

To use a different config file:

```bash
ros2 launch pcd_map_publisher pcd_map_publisher.launch.py \
  publishers_config:=/absolute/path/to/pcd_map_publishers.yaml
```

To reuse the legacy single-publisher config:

```bash
ros2 launch pcd_map_publisher pcd_map_publisher.launch.py \
  publishers_config:=/absolute/path/to/pcd_map_publisher.yaml
```
