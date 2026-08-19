# pcd_map_publisher

Simple ROS 2 package that loads a `.pcd` file and publishes it as
`sensor_msgs/msg/PointCloud2`.

## Parameters

Edit `config/pcd_map_publisher.yaml`:

- `pcd_file_path`: absolute path to the PCD file to load
- `topic_name`: topic used for publishing the point cloud
- `frame_id`: frame id stored in the message header
- `publish_rate_hz`: periodic publish rate
- `use_transient_local`: enable transient local QoS for late subscribers
- `enable_voxel_downsampling`: whether to downsample the map before publishing
- `voxel_leaf_size`: voxel size in meters used for downsampling

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
