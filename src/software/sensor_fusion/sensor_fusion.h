#pragma once

#include <google/protobuf/repeated_field.h>

#include "proto/message_translation/ssl_detection.h"
#include "proto/message_translation/ssl_geometry.h"
#include "proto/message_translation/ssl_referee.h"
#include "proto/parameters.pb.h"
#include "proto/sensor_msg.pb.h"
#include "software/sensor_fusion/filter/ball_filter.h"
#include "software/sensor_fusion/filter/robot_team_filter.h"
#include "software/sensor_fusion/filter/vision_detection.h"
#include "software/sensor_fusion/possession/possession_tracker.h"
#include "software/world/ball.h"
#include "software/world/team.h"
#include "software/world/world.h"

/**
 * Sensor Fusion is an abstraction around all filtering operations that our system may
 * need to perform. It produces Worlds that may be used, and consumes SensorProtos
 */
class SensorFusion
{
   public:
    /**
     * Creates a SensorFusion with a sensor_fusion_config
     *
     * @param sensor_fusion_config The config to fetch parameters from
     */
    explicit SensorFusion(TbotsProto::SensorFusionConfig sensor_fusion_config);

    virtual ~SensorFusion() = default;

    /**
     * Processes a new SensorProto, which may update the latest representation of the
     * World
     *
     * @param new data
     */
    void processSensorProto(const SensorProto &sensor_msg);

    /**
     * Returns the most up-to-date world if enough data has been received
     * to create one.
     *
     * @return the most up-to-date world if enough data has been received
     * to create one.
     */
    std::optional<World> getWorld() const;

    /**
     * Set the virtual obstacles in the world
     *
     * @param virtual_obstacles a list of virtual obstacles
     */
    void setVirtualObstacles(TbotsProto::VirtualObstacles virtual_obstacles);

    // Number of vision packets to indicate that the vision client most likely reset,
    // determined experimentally with the simulator
    static constexpr unsigned int VISION_PACKET_RESET_COUNT_THRESHOLD = 5;
    // Vision packets before this threshold time indicate that the vision client has just
    // started, determined experimentally with the simulator
    static constexpr double VISION_PACKET_RESET_TIME_THRESHOLD = 0.5;

    // This is used for detecting if the breakbeam is at fault. The idea is that distance
    // between the robot that has the ball is greater than the distance below, then we
    // know that the breakbeam has a problem see for more:
    // https://github.com/UBC-Thunderbots/Software/issues/3197
    static constexpr double DISTANCE_THRESHOLD_FOR_BREAKBEAM_FAULT_DETECTION = 0.5;

   private:
    /**
     * Updates relevant components of world based on a new data
     *
     * @param new data
     */
    void updateWorld(const SSLProto::SSL_WrapperPacket &packet);
    void updateWorld(const SSLProto::Referee &packet);
    void updateWorld(const google::protobuf::RepeatedPtrField<TbotsProto::RobotStatus>
                         &robot_status_msgs);
    void updateWorld(const SSLProto::SSL_GeometryData &geometry_packet);
    void updateWorld(const SSLProto::SSL_DetectionFrame &ssl_detection_frame);

    /**
     * Updates relevant components with a new ball
     *
     * @param new_ball_state new Ball
     */
    void updateBall(Ball new_ball);

    /**
     * Create state of the ball from a list of ball detections
     *
     * @param ball_detections list of ball detections to filter
     *
     * @return Ball if filtered from ball detections
     */
    std::optional<Ball> createBall(const std::vector<BallDetection> &ball_detections);

    /**
     * Create team from a list of robot detections
     *
     * @param robot_detections The robot detections to filter
     *
     * @return team
     */
    Team createFriendlyTeam(const std::vector<RobotDetection> &robot_detections);
    Team createEnemyTeam(const std::vector<RobotDetection> &robot_detections);


    /**
     * Get the ball placement point in our reference frame.
     *
     * @param referee_packet referee packet to parse
     *
     * @returns ball placement point in the team's reference frame, nothing if the packet
     * does not contain a valid point
     */
    std::optional<Point> getBallPlacementPoint(const SSLProto::Referee &packet);

    /**
     *Inverts all positions and orientations across the x and y axis
     *
     * @param Detection to invert
     *
     *@return inverted Detection
     */
    RobotDetection invert(RobotDetection robot_detection) const;
    BallDetection invert(BallDetection ball_detection) const;

    /**
     * Updates the segment representing the displacement of the ball due to
     * the friendly team continuously dribbling the ball across the field.
     */
    void updateDribbleDisplacement();

    /**
     * Checks for a vision reset and if there is one, then reset SensorFusion
     *
     * @param t_capture The t_capture of a new packet
     * @returns true if a reset is detected
     */
    bool checkForVisionReset(double t_capture);

    /**
     * Resets the world components to initial state
     */
    void resetWorldComponents();

    /**
     * Determines if the team has control over the given ball
     *
     * @param team The team to check
     * @param ball The ball to check
     *
     * @return whether the team has control over the ball
     */
    static bool teamHasBall(const Team &team, const Ball &ball);

    /**
     * This function controls the behavior of how the ball is being updated. If this
     * returns True, we use the position of the robot that triggers the breakbeam instead
     * of the one given by the ssl vision system.
     *
     * @return True if we were to use the position of the robot instead of the ssl camera
     * system. False otherwise
     */
    bool shouldTrustRobotStatus();
    TbotsProto::SensorFusionConfig sensor_fusion_config;
    std::optional<Field> field;
    std::optional<Ball> ball;
    Team friendly_team;
    Team enemy_team;
    GameState game_state;
    std::optional<RefereeStage> referee_stage;

    // Points on the field where a friendly bot initially touched the ball
    std::map<RobotId, Point> ball_contacts_by_friendly_robots;
    // Segment representing the displacement of the ball (in metres) due to
    // the friendly team continuously dribbling the ball across the field.
    //
    // - The start point of the segment is the point on the field where the friendly
    //   team started dribbling the ball.
    //
    // - The end point of the segment is the current position of the ball.
    //
    // - The length of the segment is the distance between where the friendly team
    //   started dribbling the ball and where the ball is now.
    //
    // If the friendly team does not have possession over the ball, this is std::nullopt.
    std::optional<Segment> dribble_displacement;


    BallFilter ball_filter;
    RobotTeamFilter friendly_team_filter;
    RobotTeamFilter enemy_team_filter;

    TeamPossession possession;
    std::shared_ptr<PossessionTracker> possession_tracker;

    std::optional<RobotId> friendly_robot_id_with_ball_in_dribbler;

    unsigned int friendly_goalie_id;
    unsigned int enemy_goalie_id;
    bool defending_positive_side;
    int ball_in_dribbler_timeout;

    // The number of "reset packets" we have received. These indicate that the
    // vision time should be reset. Please see `checkForVisionReset` to see how
    // this is defined.
    unsigned int reset_time_vision_packets_detected;

    // The timestamp, in seconds, of the most recently received vision packet
    double last_t_capture;

    TbotsProto::VirtualObstacles virtual_obstacles_;
};
