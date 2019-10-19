#ifndef PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_NAVIGATION_RELAYS_H_
#define PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_NAVIGATION_RELAYS_H_

#include <geometry_msgs/PointStamped.h>
#include <libsbp_ros_msgs/conversion.h>
#include <ros/ros.h>
#include <optional>
#include "piksi_multi_cpp/sbp_callback_handler/sbp_callback_handler_relay/ros_navigation_relay.h"

// This file contains specific implementations of navigation relays, i.e.,
// position, baseline, and velocity publishing.

namespace piksi_multi_cpp {

// This class publishes the most relevant position topics.
class RosPosEcefRelay
    : public RosNavigationRelay<msg_pos_ecef_t, geometry_msgs::PointStamped> {
 public:
  inline RosPosEcefRelay(const ros::NodeHandle& nh, const uint16_t sbp_msg_type,
                         const std::shared_ptr<sbp_state_t>& state,
                         const std::shared_ptr<UtcTimeBuffer>& utc_time_buffer)
      : RosNavigationRelay(nh, sbp_msg_type, state, "pos_ecef", utc_time_buffer,
                           "ecef") {}

 private:
  void convertSbpMsgToRosMsg(const msg_pos_ecef_t& in, const uint8_t len,
                             geometry_msgs::PointStamped* out) override;

};  // namespace piksi_multi_cpp

}  // namespace piksi_multi_cpp

#endif  // PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_NAVIGATION_RELAYS_H_