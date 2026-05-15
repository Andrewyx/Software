#include "software/sensor_fusion/filter/robot_filter.h"
#include <algorithm>
#include <cmath>

void RobotFilter::initMatrices()
{
    Q = Eigen::MatrixXf::Identity(STATE_DIM, STATE_DIM);
    Q(0,0) = 0.5; Q(1,1) = 0.5;    // px, py
    Q(2,2) = 10.0; Q(3,3) = 10.0;  // vx, vy
    Q(4,4) = 0.5;                  // theta
    Q(5,5) = 15.0;                 // omega

    R = Eigen::MatrixXf::Identity(MEASUREMENT_DIM, MEASUREMENT_DIM);
    R(0,0) = 0.01; R(1,1) = 0.01;  // px, py
    R(2,2) = 0.05;                 // theta

    H = Eigen::MatrixXf::Zero(MEASUREMENT_DIM, STATE_DIM);
    H(0,0) = 1.0;
    H(1,1) = 1.0;
    H(2,4) = 1.0;
}

RobotFilter::RobotFilter(Robot current_robot_state, Duration expiry_buffer_duration)
    : current_robot_state(current_robot_state),
      expiry_buffer_duration(expiry_buffer_duration)
{
    initMatrices();
    FilterState initial_state;
    initial_state.timestamp = current_robot_state.timestamp();
    initial_state.x = Eigen::VectorXf::Zero(STATE_DIM);
    initial_state.x << current_robot_state.position().x(),
                       current_robot_state.position().y(),
                       current_robot_state.velocity().x(),
                       current_robot_state.velocity().y(),
                       current_robot_state.orientation().clamp().toRadians(),
                       current_robot_state.angularVelocity().clamp().toRadians();
    initial_state.P = Eigen::MatrixXf::Identity(STATE_DIM, STATE_DIM) * 0.1;
    initial_state.measurement = std::nullopt;
    horizon_buffer.push_back(initial_state);
}

RobotFilter::RobotFilter(RobotDetection current_robot_state,
                         Duration expiry_buffer_duration)
    : current_robot_state(current_robot_state.id, current_robot_state.position,
                          Vector(0, 0), current_robot_state.orientation,
                          AngularVelocity::zero(), current_robot_state.timestamp),
      expiry_buffer_duration(expiry_buffer_duration)
{
    initMatrices();
    FilterState initial_state;
    initial_state.timestamp = current_robot_state.timestamp;
    initial_state.x = Eigen::VectorXf::Zero(STATE_DIM);
    initial_state.x << current_robot_state.position.x(),
                       current_robot_state.position.y(),
                       0.0,
                       0.0,
                       current_robot_state.orientation.clamp().toRadians(),
                       0.0;
    initial_state.P = Eigen::MatrixXf::Identity(STATE_DIM, STATE_DIM) * 0.1;
    initial_state.measurement = current_robot_state;
    horizon_buffer.push_back(initial_state);
}

void RobotFilter::predict(FilterState& state, double dt)
{
    if (dt <= 0) return;

    Eigen::MatrixXf F = Eigen::MatrixXf::Identity(STATE_DIM, STATE_DIM);
    F(0, 2) = dt;
    F(1, 3) = dt;
    F(4, 5) = dt;

    state.x = F * state.x;
    state.x(4) = Angle::fromRadians(state.x(4)).clamp().toRadians(); // wrap angle

    state.P = F * state.P * F.transpose() + Q * dt;
}

bool RobotFilter::update(FilterState& state, const RobotDetection& measurement)
{
    Eigen::VectorXf z(MEASUREMENT_DIM);
    z << measurement.position.x(), measurement.position.y(), measurement.orientation.clamp().toRadians();

    Eigen::VectorXf y = z - H * state.x;
    y(2) = Angle::fromRadians(y(2)).clamp().toRadians(); // wrap angle residual

    Eigen::MatrixXf S = H * state.P * H.transpose() + R;
    
    // Mahalanobis distance for outlier rejection
    double mahalanobis_dist = y.transpose() * S.inverse() * y;
    if (mahalanobis_dist > MAHALANOBIS_THRESHOLD)
    {
        return false; // Reject outlier
    }

    Eigen::MatrixXf K = state.P * H.transpose() * S.inverse();
    state.x = state.x + K * y;
    state.x(4) = Angle::fromRadians(state.x(4)).clamp().toRadians(); // wrap angle

    Eigen::MatrixXf I = Eigen::MatrixXf::Identity(STATE_DIM, STATE_DIM);
    state.P = (I - K * H) * state.P;

    return true;
}

void RobotFilter::pruneBuffer(Timestamp latest_timestamp)
{
    if (horizon_buffer.empty()) return;

    if (latest_timestamp.toMilliseconds() <= HORIZON_BUFFER_DURATION_MILLISECONDS)
    {
        return; // Time hasn't advanced past the horizon window yet
    }

    Timestamp cutoff = Timestamp::fromMilliseconds(
        latest_timestamp.toMilliseconds() - HORIZON_BUFFER_DURATION_MILLISECONDS);

    auto it = horizon_buffer.begin();
    while (it != horizon_buffer.end() && it->timestamp < cutoff && horizon_buffer.size() > 1)
    {
        it++;
    }
    
    if (it != horizon_buffer.begin())
    {
        // Keep the state just before the cutoff as the start of the horizon
        it--; 
        horizon_buffer.erase(horizon_buffer.begin(), it);
    }
}

std::vector<RobotDetection> RobotFilter::averageDetectionsWithSameTimestamp(
    std::vector<RobotDetection> detections)
{
    if (detections.empty()) return detections;

    std::vector<RobotDetection> averaged_detections;
    std::sort(detections.begin(), detections.end(),
              [](const RobotDetection& a, const RobotDetection& b) {
                  return a.timestamp < b.timestamp;
              });

    RobotDetection current_avg = detections.front();
    int count = 1;

    for (size_t i = 1; i < detections.size(); ++i)
    {
        if (detections[i].timestamp == current_avg.timestamp)
        {
            current_avg.position = current_avg.position + detections[i].position.toVector();
            current_avg.orientation = current_avg.orientation + detections[i].orientation;
            current_avg.confidence += detections[i].confidence;
            count++;
        }
        else
        {
            current_avg.position = Point(current_avg.position.toVector() / count);
            current_avg.orientation = current_avg.orientation / count;
            current_avg.confidence /= count;
            averaged_detections.push_back(current_avg);

            current_avg = detections[i];
            count = 1;
        }
    }
    
    current_avg.position = Point(current_avg.position.toVector() / count);
    current_avg.orientation = current_avg.orientation / count;
    current_avg.confidence /= count;
    averaged_detections.push_back(current_avg);

    return averaged_detections;
}

std::optional<Robot> RobotFilter::getFilteredData(
    const std::vector<RobotDetection>& new_robot_data,
    const std::optional<RobotId> breakbeam_tripped_id)
{
    Timestamp latest_packet_timestamp = Timestamp::fromSeconds(0);
    std::vector<RobotDetection> relevant_detections;

    for (const RobotDetection& robot_data : new_robot_data)
    {
        if (latest_packet_timestamp < robot_data.timestamp)
        {
            latest_packet_timestamp = robot_data.timestamp;
        }

        if (robot_data.id == this->getRobotId())
        {
            relevant_detections.push_back(robot_data);
        }
    }

    relevant_detections = averageDetectionsWithSameTimestamp(relevant_detections);

    bool buffer_modified = false;
    size_t rewind_start_idx = horizon_buffer.size();

    for (const auto& detection : relevant_detections)
    {
        // Discard if older than the start of the horizon buffer
        if (!horizon_buffer.empty() && detection.timestamp < horizon_buffer.front().timestamp)
        {
            continue;
        }

        // Find insertion point
        auto it = std::lower_bound(horizon_buffer.begin(), horizon_buffer.end(), detection,
            [](const FilterState& state, const RobotDetection& det) {
                return state.timestamp < det.timestamp;
            });

        size_t idx = std::distance(horizon_buffer.begin(), it);

        if (it != horizon_buffer.end() && it->timestamp == detection.timestamp)
        {
            // Already have a state here, update measurement if we want, but averaging already handled duplicates.
            // If it's a completely new measurement at the same time, we'll replace it.
            it->measurement = detection;
            buffer_modified = true;
            rewind_start_idx = std::min(rewind_start_idx, idx);
        }
        else
        {
            // Insert new state
            FilterState new_state;
            new_state.timestamp = detection.timestamp;
            new_state.measurement = detection;
            // x and P will be populated during replay
            horizon_buffer.insert(it, new_state);
            buffer_modified = true;
            rewind_start_idx = std::min(rewind_start_idx, idx);
        }
    }

    if (buffer_modified)
    {
        // Replay from rewind_start_idx to end
        // rewind_start_idx is guaranteed to be >= 1 if we maintain at least one initial state
        size_t start_idx = std::max(size_t(1), rewind_start_idx);

        for (size_t i = start_idx; i < horizon_buffer.size(); ++i)
        {
            horizon_buffer[i].x = horizon_buffer[i-1].x;
            horizon_buffer[i].P = horizon_buffer[i-1].P;

            double dt = (horizon_buffer[i].timestamp - horizon_buffer[i-1].timestamp).toSeconds();
            predict(horizon_buffer[i], dt);

            if (horizon_buffer[i].measurement.has_value())
            {
                bool accepted = update(horizon_buffer[i], horizon_buffer[i].measurement.value());
                if (!accepted)
                {
                    // If rejected as outlier, clear the measurement so we don't apply it again on future rewinds
                    horizon_buffer[i].measurement = std::nullopt;
                }
            }
        }
    }

    // Determine current global time based on latest_packet_timestamp or buffer
    Timestamp current_time = latest_packet_timestamp;
    if (!horizon_buffer.empty() && horizon_buffer.back().timestamp > current_time)
    {
        current_time = horizon_buffer.back().timestamp;
    }

    pruneBuffer(current_time);

    // Expiry check
    // If the latest state in our buffer (which is our last known robot state) is older than expiry duration,
    // we consider the robot expired.
    if (!horizon_buffer.empty())
    {
        Timestamp last_robot_timestamp = horizon_buffer.back().timestamp;
        if (current_time.toMilliseconds() >
            expiry_buffer_duration.toMilliseconds() + last_robot_timestamp.toMilliseconds())
        {
            return std::nullopt;
        }

        FilterState& final_state = horizon_buffer.back();
        bool breakbeam_tripped = breakbeam_tripped_id == getRobotId();

        this->current_robot_state =
            Robot(this->getRobotId(), 
                  Point(final_state.x(0), final_state.x(1)), 
                  Vector(final_state.x(2), final_state.x(3)),
                  Angle::fromRadians(final_state.x(4)), 
                  AngularVelocity::fromRadians(final_state.x(5)),
                  final_state.timestamp, breakbeam_tripped);
        
        return std::make_optional(this->current_robot_state);
    }

    return std::nullopt;
}

unsigned int RobotFilter::getRobotId() const
{
    return this->current_robot_state.id();
}
