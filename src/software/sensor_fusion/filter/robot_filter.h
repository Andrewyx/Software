#pragma once

#include <Eigen/Dense>
#include <optional>
#include <vector>

#include "software/geom/angle.h"
#include "software/geom/point.h"
#include "software/sensor_fusion/filter/vision_detection.h"
#include "software/time/timestamp.h"
#include "software/world/robot.h"

struct FilterState
{
    Timestamp timestamp;
    Eigen::VectorXf x;  // [px, py, vx, vy, theta, omega]^T
    Eigen::MatrixXf P;  // 6x6 covariance
    std::optional<RobotDetection>
        measurement;  // Measurement applied at this step, if any
};

class RobotFilter
{
   public:
    static constexpr int STATE_DIM                               = 6;
    static constexpr int MEASUREMENT_DIM                         = 3;
    static constexpr double HORIZON_BUFFER_DURATION_MILLISECONDS = 100.0;
    static constexpr double MAHALANOBIS_THRESHOLD = 11.34;  // 99% confidence for 3 DOF

    /**
     * Creates a new robot filter
     *
     * @param current_robot_state the data of current state of the robot
     * @param expiry_buffer_duration the time when the robot is determined to be removed
     * from the field if data about the robot is not received before that time
     */
    explicit RobotFilter(Robot current_robot_state, Duration expiry_buffer_duration);
    explicit RobotFilter(RobotDetection current_robot_state,
                         Duration expiry_buffer_duration);

    /**
     * Updates the filter given a new set of data, and returns the most up to date
     * filtered data for the Robot.
     *
     * @param new_robot_data A list of SSLRobot detections containing new robot data.
     * The data does not all have to be for a particular Robot, the filter will only use
     * the new Robot data that matches the robot id the filter was constructed with.
     *
     * @param breakbeam_tripped_id The id of the robot with the tripped breakbeam
     * according to sensor fusion filtering logic (or none if no robot has a tripped
     * beam).
     *
     * @return The filtered data for the robot
     */
    std::optional<Robot> getFilteredData(
        const std::vector<RobotDetection>& new_robot_data,
        const std::optional<RobotId> breakbeam_tripped_id = std::nullopt);

    /**
     * Returns the id of the Robot that this filter is filtering for
     *
     * @return the id of the Robot that this filter is filtering for
     */
    unsigned int getRobotId() const;

   private:
    void initMatrices();
    void predict(FilterState& state, double dt);
    bool update(FilterState& state, const RobotDetection& measurement);
    void pruneBuffer(Timestamp latest_timestamp);
    std::vector<RobotDetection> averageDetectionsWithSameTimestamp(
        std::vector<RobotDetection> detections);

    Robot current_robot_state;
    Duration expiry_buffer_duration;
    std::vector<FilterState> horizon_buffer;

    // EKF Matrices
    Eigen::MatrixXf Q;  // Process noise
    Eigen::MatrixXf R;  // Measurement noise
    Eigen::MatrixXf H;  // Measurement model
};
