#include "piksi_multi_cpp/sbp_callback_handler/geotf_handler.h"

#include <libsbp_ros_msgs/ros_conversion.h>
#include <functional>

namespace piksi_multi_cpp {
namespace s = std::placeholders;

GeoTfHandler::GeoTfHandler(const ros::NodeHandle& nh,
                           const std::shared_ptr<sbp_state_t>& state)
    : pos_llh_handler_{std::bind(&GeoTfHandler::callbackToPosLlh, this, s::_1,
                                 s::_2),
                       SBP_MSG_POS_LLH, state},
      nh_(nh) {
  geotf_.initFromRosParam();
  if (geotf_.hasFrame("enu")) reset_position_ = ResetEnuOrigin::kNo;
  ROS_ERROR_COND(
      !geotf_.addFrameByEPSG("ecef", 4978),
      "Failed to add frame ecef as EPSG:4978. Be careful using converted "
      "positions");
  ROS_ERROR_COND(
      !geotf_.addFrameByEPSG("wgs84", 4326),
      "Failed to add frame ecef as EPSG:4326. Be careful using converted "
      "positions");

  set_enu_origin_srv_ = nh_.advertiseService(
      "set_enu_origin", &GeoTfHandler::setEnuOriginCallback, this);
  set_enu_from_base_srv_ =
      nh_.advertiseService("set_enu_origin_from_base_station",
                           &GeoTfHandler::setEnuOriginFromBaseStation, this);
  set_enu_from_current_srv_ =
      nh_.advertiseService("set_enu_origin_from_current_pos",
                           &GeoTfHandler::setEnuOriginFromCurrentPos, this);

  ros::NodeHandle nh_private("~");
  base_pos_sub_ =
      nh_private.subscribe("position_receiver_0/sbp/base_pos_ecef", 1,
                           &GeoTfHandler::callbackToBasePosEcef, this);

  bool set_enu_origin_from_current_pos =
      reset_position_ == ResetEnuOrigin::kFromCurrentPos;
  nh_private.param("set_enu_origin_from_current_pos",
                   set_enu_origin_from_current_pos,
                   set_enu_origin_from_current_pos);
  if (set_enu_origin_from_current_pos)
    reset_position_ = ResetEnuOrigin::kFromCurrentPos;
}

void GeoTfHandler::setEnuOriginWgs84(const double lat, const double lon,
                                     const double alt) {
  geotf_.removeFrame("enu");
  geotf_.addFrameByENUOrigin("enu", lat, lon, alt);
  reset_position_ = ResetEnuOrigin::kNo;
}

void GeoTfHandler::callbackToBasePosEcef(
    const libsbp_ros_msgs::MsgBasePosEcef::Ptr& msg) {
  if (reset_position_ != ResetEnuOrigin::kFromBase) return;
  Eigen::Vector3d x_ecef, x_wgs84;
  x_ecef << msg->x, msg->y, msg->z;
  if (!geotf_.convert("ecef", x_ecef, "wgs84", &x_wgs84)) {
    ROS_ERROR("Cannot convert ECEF to WGS84.");
    ROS_ERROR("Cannot set ENU origin.");
    return;
  }
  setEnuOriginWgs84(x_wgs84.x(), x_wgs84.y(), x_wgs84.z());
}

void GeoTfHandler::callbackToPosLlh(const msg_pos_llh_t& msg,
                                    const uint8_t len) {
  if (reset_position_ != ResetEnuOrigin::kFromCurrentPos) return;
  if (((msg.flags >> 0) & 0x7) == 0) return;  // Fix invalid.
  setEnuOriginWgs84(msg.lat, msg.lon, msg.height);
}

bool GeoTfHandler::setEnuOriginCallback(
    piksi_rtk_msgs::EnuOrigin::Request& req,
    piksi_rtk_msgs::EnuOrigin::Response& res) {
  setEnuOriginWgs84(req.lat, req.lon, req.alt);
  return true;
}

bool GeoTfHandler::setEnuOriginFromBaseStation(std_srvs::Empty::Request& req,
                                               std_srvs::Empty::Response& res) {
  reset_position_ = ResetEnuOrigin::kFromBase;
  return true;
}

bool GeoTfHandler::setEnuOriginFromCurrentPos(std_srvs::Empty::Request& req,
                                              std_srvs::Empty::Response& res) {
  reset_position_ = ResetEnuOrigin::kFromCurrentPos;
  return true;
}

}  // namespace piksi_multi_cpp
