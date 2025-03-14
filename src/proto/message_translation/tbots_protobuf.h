#pragma once

#include "proto/message_translation/tbots_geometry.h"
#include "proto/tbots_software_msgs.pb.h"
#include "proto/vision.pb.h"
#include "proto/visualization.pb.h"
#include "proto/world.pb.h"
#include "software/ai/navigator/trajectory/bang_bang_trajectory_1d_angular.h"
#include "software/ai/navigator/trajectory/trajectory_path.h"
#include "software/ai/passing/pass_with_rating.h"
#include "software/world/world.h"

/**
 * Returns a TbotsProto::World proto given a World.
 *
 * @param world The world msg to extract the TbotsProto::World from
 *
 * @return The unique_ptr to a TbotsProto::World proto containing the field, friendly
 * team, enemy team, ball, and the game state.
 */
std::unique_ptr<TbotsProto::World> createWorld(const World& world);

/**
 * Returns a TbotsProto::World proto with a sequence number given a World and a sequence
 * number.
 *
 * @param world The world msg to extract the TbotsProto::World from
 * @param sequence_number A sequence number for tracking the TbotsProto::World
 *
 * @return The unique_ptr to a TbotsProto::World proto containing the field, friendly
 * team, enemy team, ball, game state, and the sequence number.
 */
std::unique_ptr<TbotsProto::World> createWorldWithSequenceNumber(
    const World& world, const uint64_t sequence_number);

/**
 * Returns a TbotsProto::Team proto given a Team.
 *
 * @param team The Team msg to extract the TbotsProto::Team from
 *
 * @return The unique_ptr to a TbotsProto::Team proto containing a list of robots and
 * goalie ID
 */
std::unique_ptr<TbotsProto::Team> createTeam(const Team& team);

/**
 * Returns a TbotsProto::Robot proto given a Robot.
 *
 * @param robot The Robot msg to extract the TbotsProto::Robot from
 *
 * @return The unique_ptr to a TbotsProto::Robot proto containing the robot ID and robot
 * state
 */
std::unique_ptr<TbotsProto::Robot> createRobot(const Robot& robot);

/**
 * Returns a TbotsProto::Ball proto given a Ball.
 *
 * @param ball The Ball msg to extract the TbotsProto::Ball from
 *
 * @return The unique_ptr to a TbotsProto::Ball proto containing the ball state and the
 * ball acceleration
 */
std::unique_ptr<TbotsProto::Ball> createBall(const Ball& ball);

/**
 * Returns a TbotsProto::Field proto given a Field.
 *
 * @param field The Field msg to extract the TbotsProto::Field from
 *
 * @return The unique_ptr to a TbotsProto::Field proto containing the Ball ID and Ball
 * state
 */
std::unique_ptr<TbotsProto::Field> createField(const Field& field);

/**
 * Returns (Robot, Game, Ball) State given a (Robot, Game, Ball)
 *
 * @param The (Robot, Game, Ball) to convert to State proto
 *
 * @return The unique_ptr to a (Robot, Game, Ball) State after conversion
 */
std::unique_ptr<TbotsProto::RobotState> createRobotStateProto(const Robot& robot);
std::unique_ptr<TbotsProto::RobotState> createRobotStateProto(
    const RobotState& robot_state);
std::unique_ptr<TbotsProto::GameState> createGameState(const GameState& game_state);
std::unique_ptr<TbotsProto::BallState> createBallState(const Ball& ball);

/**
 * Returns a TbotsProto::Timestamp proto given a timestamp.
 *
 * @param timestamp The Timestamp msg to extract the TbotsProto::Timestamp from
 *
 * @return The unique_ptr to a TbotsProto::Timestamp proto containing the timestamp with
 * the same time zone as the timestamp argument.
 */
std::unique_ptr<TbotsProto::Timestamp> createTimestamp(const Timestamp& timestamp);

/**
 * Returns a TbotsProto::NamedValue proto given a name and value.
 *
 * @param name The name of the value to plot
 * @param value The NamedValue msg to extract the TbotsProto::NamedValue from
 *
 * @return The unique_ptr to a TbotsProto::NamedValue proto containing data with
 *         specified name and value
 */
std::unique_ptr<TbotsProto::NamedValue> createNamedValue(const std::string name,
                                                         float value);

/**
 * Returns a TbotsProto::PlotJugglerValue proto containing the name
 * value pairs of the map.
 *
 * Could use LOG(PLOTJUGGLER) to plot the values. Example:
 *  LOG(PLOTJUGGLER) << *createPlotJugglerValue({
 *      {"vx", velocity.x()},
 *      {"vy", velocity.y()}
 *  });
 *
 * @param values The map of name value pairs to plot
 *
 * @return The unique_ptr to a TbotsProto::PlotJugglerValue proto containing data with
 *        specified names and values
 */
std::unique_ptr<TbotsProto::PlotJugglerValue> createPlotJugglerValue(
    const std::map<std::string, double>& values);

/**
 * Returns a TbotsProto::DebugShapes::DebugShape proto for a given shape,
 * unique id, and an optional debug text.
 *
 * @tparam Shape A shape which has a matching createShapeProto function
 *
 * @param shape The shape to create the protobuf for
 * @param unique_id A unique string used to differentiate this shape
 * from others. Note that this ID should remain the same for the same shape
 * to avoid it from being plotted multiple times.
 * @param debug_text An optional string to display on the shape
 *
 * @return The unique_ptr to a TbotsProto::DebugShapes::DebugShape proto
 */
template <class Shape>
std::unique_ptr<TbotsProto::DebugShapes::DebugShape> createDebugShape(
    const Shape& shape, const std::string& unique_id, const std::string& debug_text = "")
{
    auto debug_shape = std::make_unique<TbotsProto::DebugShapes::DebugShape>();
    (*debug_shape->mutable_shape()) = *createShapeProto(shape);
    debug_shape->set_unique_id(unique_id);
    debug_shape->set_debug_text(debug_text);
    return debug_shape;
};

/**
 * Returns a TbotsProto::DebugShapes proto containing the debug shapes
 *
 * Could use LOG(VISUALIZE) to plot these values. Example:
 *  LOG(VISUALIZE) << *createDebugShapes({
 *       *createDebugShape(circle, unique_id1, optional_text),
 *       *createDebugShape(polygon, unique_id2, optional_text),
 *       *createDebugShape(stadium, unique_id3, optional_text)
 *  });
 *
 * @param debug_shapes A list of debug shapes proto to plot
 *
 * @return The unique_ptr to a TbotsProto::DebugShapes proto containing data with
 *        specified names and shapes
 */
std::unique_ptr<TbotsProto::DebugShapes> createDebugShapes(
    const std::vector<TbotsProto::DebugShapes::DebugShape>& debug_shapes);

/**
 * Returns a TbotsProto::Shape proto given a shape.
 *
 * @param shape The shape to create a TbotsProto::Shape for
 *
 * @return The unique_ptr to a TbotsProto::Shape proto containing the shape
 */
std::unique_ptr<TbotsProto::Shape> createShapeProto(const Circle& circle);
std::unique_ptr<TbotsProto::Shape> createShapeProto(const Polygon& polygon);
std::unique_ptr<TbotsProto::Shape> createShapeProto(const Stadium& stadium);

/**
 * Returns a timestamp msg with the time that this function was called
 *
 * @return The unique_ptr to a TbotsProto::Timestamp with the current UTC time
 */
std::unique_ptr<TbotsProto::Timestamp> createCurrentTimestamp();

/**
 * Return RobotState given the TbotsProto::RobotState protobuf
 *
 * @param robot_state The RobotState proto to create a RobotState from
 * @return the RobotState
 */
RobotState createRobotState(const TbotsProto::RobotState robot_state);

/**
 * Return BallState given the TbotsProto::BallState protobuf
 *
 * @param robot_state The BallState proto to create a RobotState from
 * @return the BallState
 */
BallState createBallState(const TbotsProto::BallState ball_state);

/**
 * Returns a pass visualization given a vector of the best passes
 *
 * @param A vector of passes across their fields  with their ratings
 *
 * @return The unique_ptr to a PassVisualization proto
 */
std::unique_ptr<TbotsProto::PassVisualization> createPassVisualization(
    const std::vector<PassWithRating>& passes_with_rating);

/**
 * Returns the WorldStateReceivedTrigger given the world state received trigger
 *
 * @return The unique_ptr to a TbotsProto::WorldStateReceivedTrigger proto containing
 *         a boolean value for whether world state proto has been received
 *
 */
std::unique_ptr<TbotsProto::WorldStateReceivedTrigger> createWorldStateReceivedTrigger();

/**
 * Returns a cost visualization given a vector of costs
 *
 * @param costs A vector of costs to visualize
 * @param num_rows The number of rows to display in the cost visualization
 * @param num_cols The number of columns to display in the cost visualization
 *
 * @return The unique_ptr to a CostVisualization proto
 */
std::unique_ptr<TbotsProto::CostVisualization> createCostVisualization(
    const std::vector<double>& costs, int num_rows, int num_cols);

/**
 * Generate a 2D Trajectory Path given 2D trajectory parameters
 *
 * @param params 2D Trajectory Path
 * @param initial_velocity Initial velocity to use for the trajectory
 * @param robot_constants Constants to use for the trajectory
 * @return TrajectoryPath, or std::nullopt if the trajectory path could not be created
 * from the given parameters
 */
std::optional<TrajectoryPath> createTrajectoryPathFromParams(
    const TbotsProto::TrajectoryPathParams2D& params, const Vector& initial_velocity,
    const RobotConstants& robot_constants);

/**
 * Generate an angular trajectory Path given angular trajectory proto parameters
 *
 * @param params angular Trajectory Path
 * @param initial_velocity Initial velocity to use for the trajectory
 * @param robot_constants Constants to use for the trajectory
 * @return Generate angular trajectory
 */
BangBangTrajectory1DAngular createAngularTrajectoryFromParams(
    const TbotsProto::TrajectoryParamsAngular1D& params,
    const AngularVelocity& initial_velocity, const RobotConstants& robot_constants);

/**
 * Convert dribbler mode to dribbler speed
 *
 * @param dribbler_mode The DribblerMode
 * @param robot_constants The robot constants
 *
 * @return the dribbler speed in RPM
 */
int convertDribblerModeToDribblerSpeed(TbotsProto::DribblerMode dribbler_mode,
                                       RobotConstants_t robot_constants);

/**
 * Convert max allowed speed mode to max allowed speed
 *
 * @param max_allowed_speed_mode The MaxAllowedSpeedMode
 * @param robot_constants The robot constants
 *
 * @return the max allowed speed in m/s
 */
double convertMaxAllowedSpeedModeToMaxAllowedSpeed(
    TbotsProto::MaxAllowedSpeedMode max_allowed_speed_mode,
    RobotConstants_t robot_constants);
