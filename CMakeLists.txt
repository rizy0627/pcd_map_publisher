cmake_minimum_required(VERSION 3.8)
project(pcd_map_publisher LANGUAGES CXX)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED COMPONENTS common filters io)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_definitions(${PCL_DEFINITIONS})

add_executable(pcd_map_publisher_node
  src/pcd_map_publisher_node.cpp
)

target_include_directories(pcd_map_publisher_node PRIVATE
  ${PCL_INCLUDE_DIRS}
)

target_link_libraries(pcd_map_publisher_node
  ${PCL_LIBRARIES}
)

ament_target_dependencies(pcd_map_publisher_node
  pcl_conversions
  rclcpp
  sensor_msgs
)

install(TARGETS
  pcd_map_publisher_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  config
  launch
  DESTINATION share/${PROJECT_NAME}
)

install(FILES
  README.md
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
