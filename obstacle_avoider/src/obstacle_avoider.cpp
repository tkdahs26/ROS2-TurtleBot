#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include <vector>
#include <algorithm>
#include <limits>
#include "geometry_msgs/msg/twist_stamped.hpp"

class ObstacleAvoider : public rclcpp::Node
{
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr publisher_;
  rclcpp::Time turn_end_{0, 0, RCL_ROS_TIME};
double turn_dir_ = 1.0;
public:ObstacleAvoider() : Node("obstacle_avoider")
  {

    subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>("/scan", 10,
      std::bind(&ObstacleAvoider::scan_callback, this, std::placeholders::_1));

    publisher_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("/cmd_vel", 10);
 

    this->declare_parameter("safe_distance", 0.5);
    this->declare_parameter("linear_speed", 0.6);
    this->declare_parameter("angular_speed", 0.5);
    this->declare_parameter("front_angle_range", 30);

    RCLCPP_INFO(this->get_logger(), "Obstacle Avoider 노드 시작!");
  }

private:void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
    double safe_distance = this->get_parameter("safe_distance").as_double();
    double linear_speed = this->get_parameter("linear_speed").as_double();
    double angular_speed = this->get_parameter("angular_speed").as_double();
    int front_range = this->get_parameter("front_angle_range").as_int();
/*
    int total = msg->ranges.size();
    int center = total / 2;
    int start = std::max(0, center - front_range);
    int end = std::min(total - 1, center + front_range);

RCLCPP_INFO(this->get_logger(),  "total=%d center=%d start=%d end=%d front_range=%d",total, center, start, end, front_range);
*/


const int n = static_cast<int>(msg->ranges.size());
//정면 벽과의거리계산
double min_distance = std::numeric_limits<double>::infinity();

//왼쪽면 오른쪽면 벽과의 거리 계산
double left_min = std::numeric_limits<double>::infinity();
double right_min = std::numeric_limits<double>::infinity(); 

//정면 인덱스 0~30 / 330~360
for (int i = 0; i <= front_range; ++i) {
  float r = msg->ranges[i];
  if (std::isfinite(r) && r > msg->range_min && r < msg->range_max) {
    if (r < min_distance) 
    min_distance = r;
  }
}

for (int i = n - front_range; i < n; ++i) {
  float r = msg->ranges[i];
  if (std::isfinite(r) && r > msg->range_min && r < msg->range_max) {
    if (r < min_distance) min_distance = r;
  }
}

//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ z값 부호 결정ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
for (int i = 1; i <= 90; ++i) {
  float r = msg->ranges[i];
  if (std::isfinite(r) && r > msg->range_min && r < msg->range_max) {
    if (r < left_min) left_min = r;
  }
}

for (int i = n - 90; i < n; ++i) {
  float r = msg->ranges[i];
  if (std::isfinite(r) && r > msg->range_min && r < msg->range_max) {
    if (r < right_min) right_min = r;
  }
}


    /*     앞을 봐야되는데 뒤를보는코드
    int total = msg->ranges.size();
    int center = total / 2;
    int start = std::max(0, center - front_range);
    int end = std::min(total - 1, center + front_range);



    double min_distance = std::numeric_limits<double>::infinity();
    RCLCPP_INFO(this->get_logger(),  "min_distance=%f ",min_distance);

    for (int i = start; i <= end; ++i) {
      float r = msg->ranges[i];
      if (std::isfinite(r) && r > msg->range_min && r < msg->range_max) {
if (r < min_distance) {
    min_distance = r;
        }
      }
    }
      */
    
geometry_msgs::msg::TwistStamped twist;
twist.header.stamp = this->now();
twist.header.frame_id = "base_link";

//RCLCPP_INFO(this->get_logger(),  "now=%.2f turn_end=%.2f",  this->now().seconds(),  turn_end_.seconds());
/*
if (this->now() < turn_end_) {
  twist.twist.linear.x = 0.00;
  twist.twist.angular.z = 0.5;
} else if (min_distance <safe_distance) {
  turn_end_ = this->now() + rclcpp::Duration::from_seconds(3.0); // turn_end_ 3초 추가 후 위 if 값 참으로 되므로 계산
   RCLCPP_INFO(this->get_logger(), " 감지: %.2f m -> 3초 회전", min_distance);
} else {
  twist.twist.linear.x = linear_speed;
  twist.twist.angular.z = 0.0;
}
*/
if (this->now() < turn_end_) {
  twist.twist.linear.x = 0.0;
  twist.twist.angular.z = angular_speed * turn_dir_; // turn_dir_  z축 부호 저장용 (양수면 왼쪽 음수면 오른쪽)
} else if (min_distance < safe_distance) {
  if (left_min > right_min) turn_dir_ = 1.0;   // 왼쪽이 더 빔
  else                      turn_dir_ = -1.0;  // 오른쪽이 더 빔

  turn_end_ = this->now() + rclcpp::Duration::from_seconds(3.0);// turn_end_ 3초 추가 후 위 if 값 참으로 되므로 계산
  twist.twist.linear.x = 0.0;
  twist.twist.angular.z = angular_speed * turn_dir_;

  RCLCPP_INFO(this->get_logger(),    "장애물 %.2f / 왼쪽=%.2f 오른쪽=%.2f -> %s",    min_distance, left_min, right_min,
    (turn_dir_ > 0.0) ? "왼쪽" : "오른쪽");


} else {
  twist.twist.linear.x = linear_speed;
  twist.twist.angular.z = 0.0;
}



publisher_->publish(twist);
  }
 
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ObstacleAvoider>());
  rclcpp::shutdown();
  return 0;
}