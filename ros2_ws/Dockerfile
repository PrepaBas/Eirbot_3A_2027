FROM osrf/ros:jazzy-desktop-full AS builder

WORKDIR /ros2_ws
COPY src/ src/
RUN colcon build
