#ifndef PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_EXT_EVENT_RELAY_H_
#define PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_EXT_EVENT_RELAY_H_

#include <libsbp/ext_events.h>
#include <piksi_rtk_msgs/ExtEvent.h>
#include "piksi_multi_cpp/sbp_callback_handler/sbp_callback_handler_relay/sbp_callback_handler_relay.h"
#include "piksi_multi_cpp/sbp_callback_handler/utc_time_buffer.h"

namespace piksi_multi_cpp {

class RosExtEventRelay
    : public SBPCallbackHandlerRelay<msg_ext_event_t,
                                     piksi_rtk_msgs::ExtEvent> {
 public:
  inline RosExtEventRelay(const ros::NodeHandle& nh,
                          const std::shared_ptr<sbp_state_t>& state,
                          const std::shared_ptr<UtcTimeBuffer>& utc_time_buffer)
      : SBPCallbackHandlerRelay<msg_ext_event_t, piksi_rtk_msgs::ExtEvent>(
            nh, SBP_MSG_EXT_EVENT, state, "ros/ext_event"),
        utc_time_buffer_(utc_time_buffer) {}

 private:
  inline piksi_rtk_msgs::ExtEvent convertSbpToRos(
      const msg_ext_event_t& sbp_msg, const uint8_t len) override {
    piksi_rtk_msgs::ExtEvent ros_msg;

    // Assign time stamp.
    if (utc_time_buffer_.get())
      // TODO(rikba): Not sure if the external time stamp will occur in the UTC
      // tmie messages. Maybe we have to manually convert the GPS time stamp to
      // UTC time.
      ros_msg.stamp.data = utc_time_buffer_->getTime(sbp_msg.tow);
    else {
      ROS_WARN_ONCE("Using ros::Time::now() to stamp external event.");
      ros_msg.stamp.data = ros::Time::now();
    }

    ros_msg.pin_value = (sbp_msg.flags >> 0) & 0x1;
    ros_msg.quality = (sbp_msg.flags >> 1) & 0x1;
    ros_msg.pin = sbp_msg.pin;

    return ros_msg;
  }

  std::shared_ptr<UtcTimeBuffer> utc_time_buffer_;
};
}  // namespace piksi_multi_cpp

#endif  // PIKSI_MULTI_CPP_SBP_CALLBACK_HANDLER_SBP_CALLBACK_HANDLER_RELAY_ROS_EXT_EVENT_RELAY_H_