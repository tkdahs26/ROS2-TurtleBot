# ROS2 TurtleBot3 장애물 회피

TurtleBot3 LiDAR(`/scan`)를 보고 `/cmd_vel`로 직진,회전하는 ROS 2 노드입니다.

- ROS 2 Jazzy, C++, Gazebo
- 전방 0.5m 이하면 회전, 아니면 직진
- 좌우 거리 비교 후 더 열린 쪽으로 회전

   자세한 설명: [노션 포트폴리오](https://bsm-portfolio.notion.site/ros-turtlebot)

## 실행
```bash
cd ~/ros2_ws
colcon build --packages-select obstacle_avoider
source install/setup.bash
ros2 run obstacle_avoider obstacle_avoider
