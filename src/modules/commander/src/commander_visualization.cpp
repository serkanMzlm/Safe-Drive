#include "commander_visualization.hpp"

void visualizationTf2(geometry_msgs::msg::TransformStamped &t, State_t state, std::string frame_id)
{
    t.header.stamp = rclcpp::Clock().now();
    t.header.frame_id = frame_id;
    t.child_frame_id = "base_link";

    t.transform.translation.x = state.position.x / 10.0;
    t.transform.translation.y = state.position.y / 10.0;
    t.transform.translation.z = state.position.z / 10.0;

    t.transform.rotation.x = state.quaternion.q[0];
    t.transform.rotation.y = state.quaternion.q[1];
    t.transform.rotation.z = state.quaternion.q[2];
    t.transform.rotation.w = state.quaternion.q[3];
}

bool visualizationPath(poseStampedMsg &pose_stamped, Position_t pose, std::string frame_id)
{
    static float prev_pose[3] = {0.0, 0.0, 0.0};

    if (prev_pose[0] == pose.x && prev_pose[1] == pose.y && prev_pose[2] == pose.z)
    {
        return false;
    }

    pose_stamped.header.frame_id = frame_id;
    pose_stamped.header.stamp = rclcpp::Clock().now();
    pose_stamped.pose.position.x = pose.x / 10.0;
    pose_stamped.pose.position.y = pose.y / 10.0;
    pose_stamped.pose.position.z = pose.z / 10.0;

    prev_pose[0] = pose.x;
    prev_pose[1] = pose.y;
    prev_pose[2] = pose.z;

    return true;
}

void visualizationMarker(markerMsg &marker, float linear_x, float angular_z, int marker_id, std::string frame_id)
{
    pointMsg start_point;
    pointMsg end_point;

    marker.header.frame_id = frame_id;
    marker.ns = "marker_" + std::to_string(marker_id);
    marker.type = markerMsg::ARROW;
    marker.id = marker_id;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = 0.04;
    marker.scale.y = 0.1;
    marker.scale.z = 0.1;
    marker.color.a = 1.0;
    marker.color.r = marker_id % 3 == 0 ? 1.0 : 0.0;
    marker.color.g = marker_id % 3 == 1 ? 1.0 : 0.0;
    marker.color.b = marker_id % 3 == 2 ? 1.0 : 0.0;
    calculateVector(linear_x, angular_z, start_point, end_point, marker_id);

    marker.points.push_back(start_point);
    marker.points.push_back(end_point);
}

void visualizationPointCloud(pointCloudMsg msg, pointCloudMsg &corrected_data, State_t state, std::string frame_id)
{
    corrected_data = msg;
    float* prev_data[3];
    bool control_flag = false;

    for (size_t row = 0; row < msg.height; ++row)
    {
        for (size_t col = 0; col < msg.width; ++col)
        {
            uint32_t index = row * msg.row_step + col * msg.point_step;
            prev_data[0] = (reinterpret_cast<float*>(&msg.data[index]));
			prev_data[1] = (reinterpret_cast<float*>(&msg.data[index + 4]));
			prev_data[2] = (reinterpret_cast<float*>(&msg.data[index + 8]));

            control_flag = std::isinf(std::abs(*prev_data[0])) ||
                           std::isnan(std::abs(*prev_data[0]));
            if (control_flag)
            {
                transformation(*prev_data, state.orientation.angle, state.position.pose);
                // *(reinterpret_cast<float *>(*msg.data[index])) = prev_data[0];
                // *(reinterpret_cast<float *>(*msg.data[index + 4])) = prev_data[1];
                // *(reinterpret_cast<float *>(*msg.data[index + 8])) = prev_data[2];
            }
        }
    }
}

void calculateVector(float linear_x, float angular_z, pointMsg &start, pointMsg &end, int id)
{
    switch (id)
    {
    case 0:
        start.x = 0.0f;
        start.y = 0.0f;
        start.z = 0.0f;
        end.x = linear_x;
        end.y = 0.0f;
        end.z = 0.0f;
        break;
    case 1:
        start.x = 0.0f;
        start.y = 0.0f;
        start.z = 0.0f;
        end.x = 0.0f;
        end.y = angular_z;
        end.z = 0.0f;
        break;
    case 2:
        start.x = 0.0;
        start.y = angular_z;
        start.z = 0.0f;
        end.x = linear_x;
        end.y = 0.0f;
        end.z = 0.0f;
        break;
    default:
        break;
    }
}