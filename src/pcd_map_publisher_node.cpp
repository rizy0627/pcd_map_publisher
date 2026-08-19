#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

#include <pcl/PCLPointCloud2.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl_conversions/pcl_conversions.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

using namespace std::chrono_literals;

class PcdMapPublisher : public rclcpp::Node
{
public:
  PcdMapPublisher()
  : Node("pcd_map_publisher")
  {
    const std::string pcd_file_path =
      this->declare_parameter<std::string>("pcd_file_path", "");
    const std::string topic_name =
      this->declare_parameter<std::string>("topic_name", "/pcd_map");
    frame_id_ = this->declare_parameter<std::string>("frame_id", "map");
    const double publish_rate_hz =
      this->declare_parameter<double>("publish_rate_hz", 1.0);
    const bool use_transient_local =
      this->declare_parameter<bool>("use_transient_local", true);
    const bool enable_voxel_downsampling =
      this->declare_parameter<bool>("enable_voxel_downsampling", false);
    const double voxel_leaf_size =
      this->declare_parameter<double>("voxel_leaf_size", 0.2);

    if (pcd_file_path.empty()) {
      throw std::invalid_argument("Parameter 'pcd_file_path' must not be empty.");
    }

    if (publish_rate_hz <= 0.0) {
      throw std::invalid_argument("Parameter 'publish_rate_hz' must be greater than zero.");
    }

    if (enable_voxel_downsampling && voxel_leaf_size <= 0.0) {
      throw std::invalid_argument("Parameter 'voxel_leaf_size' must be greater than zero.");
    }

    rclcpp::QoS qos(rclcpp::KeepLast(1));
    if (use_transient_local) {
      qos.transient_local();
    }
    publisher_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(topic_name, qos);

    load_point_cloud(
      pcd_file_path,
      enable_voxel_downsampling,
      static_cast<float>(voxel_leaf_size));

    const auto period = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::duration<double>(1.0 / publish_rate_hz));
    timer_ = this->create_wall_timer(
      period > 0ms ? period : 1ms,
      std::bind(&PcdMapPublisher::publish_point_cloud, this));

    publish_point_cloud();

    RCLCPP_INFO(
      get_logger(),
      "Publishing PCD file '%s' on topic '%s' with frame_id '%s' at %.2f Hz",
      pcd_file_path.c_str(),
      publisher_->get_topic_name(),
      frame_id_.c_str(),
      publish_rate_hz);
  }

private:
  static std::size_t point_count(const pcl::PCLPointCloud2 & cloud)
  {
    return static_cast<std::size_t>(cloud.width) * static_cast<std::size_t>(cloud.height);
  }

  static pcl::PCLPointCloud2 apply_voxel_downsampling(
    const pcl::PCLPointCloud2 & input_cloud,
    const float voxel_leaf_size)
  {
    pcl::VoxelGrid<pcl::PCLPointCloud2> voxel_grid;
    pcl::PCLPointCloud2::Ptr input_cloud_ptr(new pcl::PCLPointCloud2(input_cloud));
    pcl::PCLPointCloud2 filtered_cloud;

    voxel_grid.setInputCloud(input_cloud_ptr);
    voxel_grid.setLeafSize(voxel_leaf_size, voxel_leaf_size, voxel_leaf_size);
    voxel_grid.filter(filtered_cloud);

    return filtered_cloud;
  }

  void load_point_cloud(
    const std::string & pcd_file_path,
    const bool enable_voxel_downsampling,
    const float voxel_leaf_size)
  {
    pcl::PCLPointCloud2 pcl_cloud;
    const int load_result = pcl::io::loadPCDFile(pcd_file_path, pcl_cloud);
    if (load_result < 0) {
      throw std::runtime_error("Failed to load PCD file: " + pcd_file_path);
    }

    const std::size_t raw_point_count = point_count(pcl_cloud);
    pcl::PCLPointCloud2 processed_cloud = pcl_cloud;

    if (enable_voxel_downsampling) {
      processed_cloud = apply_voxel_downsampling(pcl_cloud, voxel_leaf_size);
      RCLCPP_INFO(
        get_logger(),
        "Applied voxel downsampling with leaf size %.3f m: %zu -> %zu points",
        voxel_leaf_size,
        raw_point_count,
        point_count(processed_cloud));
    }

    pcl_conversions::fromPCL(processed_cloud, point_cloud_msg_);
    point_cloud_msg_.header.frame_id = frame_id_;

    RCLCPP_INFO(
      get_logger(),
      "Loaded PCD file '%s' (%u x %u points, %zu fields)",
      pcd_file_path.c_str(),
      point_cloud_msg_.width,
      point_cloud_msg_.height,
      point_cloud_msg_.fields.size());
  }

  void publish_point_cloud()
  {
    point_cloud_msg_.header.stamp = this->now();
    publisher_->publish(point_cloud_msg_);
  }

  std::string frame_id_;
  sensor_msgs::msg::PointCloud2 point_cloud_msg_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  try {
    auto node = std::make_shared<PcdMapPublisher>();
    rclcpp::spin(node);
  } catch (const std::exception & exception) {
    RCLCPP_FATAL(
      rclcpp::get_logger("pcd_map_publisher"),
      "Node exited with error: %s",
      exception.what());
    rclcpp::shutdown();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}
