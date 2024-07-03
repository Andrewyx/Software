#include "software/ai/hl/stp/tactic/pass_defender/pass_defender_fsm.h"

#include "software/ai/hl/stp/tactic/move_primitive.h"
#include "software/geom/algorithms/closest_point.h"

bool PassDefenderFSM::passStarted(const Update& event)
{
    auto ball_position = event.common.world_ptr->ball().position();
    Vector ball_receiver_point_vector(
        event.control_params.position_to_block_from.x() - ball_position.x(),
        event.control_params.position_to_block_from.y() - ball_position.y());

    bool pass_started = event.common.world_ptr->ball().hasBallBeenKicked(
        ball_receiver_point_vector.orientation(), MIN_PASS_SPEED,
        MAX_PASS_ANGLE_DIFFERENCE);

    if (pass_started)
    {
        // We want to keep track of the initial trajectory of the pass
        // so that we can later tell whether the ball strays from
        // this trajectory
        pass_orientation = event.common.world_ptr->ball().velocity().orientation();
    }

    return pass_started;
}

bool PassDefenderFSM::ballDeflected(const Update& event)
{
    auto orientation_difference =
        event.common.world_ptr->ball().velocity().orientation().minDiff(pass_orientation);

    // If the ball strays from the initial trajectory of the pass,
    // it was likely deflected off course by another robot or chipped
    // away by the pass defender
    return orientation_difference.abs() > MIN_DEFLECTION_ANGLE;
}

void PassDefenderFSM::blockPass(const Update& event)
{
    auto position_to_block_from = event.control_params.position_to_block_from;
    auto ball_position          = event.common.world_ptr->ball().position();
    auto face_ball_orientation =
        (ball_position - event.common.robot.position()).orientation();

    // Face the ball and move to position_to_block_from, which should be a location
    // on the field that blocks a passing lane between two enemy robots
    event.common.set_primitive(std::make_unique<MovePrimitive>(
        event.common.robot, position_to_block_from, face_ball_orientation,
        TbotsProto::MaxAllowedSpeedMode::PHYSICAL_LIMIT,
        TbotsProto::ObstacleAvoidanceMode::AGGRESSIVE, TbotsProto::DribblerMode::OFF,
        TbotsProto::BallCollisionType::ALLOW,
        AutoChipOrKick{AutoChipOrKickMode::OFF, 0}));
}

void PassDefenderFSM::interceptBall(const Update& event)
{
    auto ball           = event.common.world_ptr->ball();
    auto robot_position = event.common.robot.position();

    if ((ball.position() - robot_position).length() >
        BALL_TO_FRONT_OF_ROBOT_DISTANCE_WHEN_DRIBBLING)
    {
        Point intercept_position = ball.position();
        if (ball.velocity().length() != 0)
        {
            // Find the closest point on the line of the ball's current trajectory
            // that the defender can move to and intercept the pass
            intercept_position = closestPoint(
                robot_position, Line(ball.position(), ball.position() + ball.velocity()));
        }

        auto face_ball_orientation = (ball.position() - robot_position).orientation();

        // Move to intercept the pass by positioning defender in front of the
        // ball's current trajectory
        event.common.set_primitive(std::make_unique<MovePrimitive>(
            event.common.robot, intercept_position, face_ball_orientation,
            TbotsProto::MaxAllowedSpeedMode::PHYSICAL_LIMIT,
            TbotsProto::ObstacleAvoidanceMode::AGGRESSIVE,
            TbotsProto::DribblerMode::MAX_FORCE, TbotsProto::BallCollisionType::ALLOW,
            AutoChipOrKick{AutoChipOrKickMode::OFF, 0}));
        return;
    }

    // The ball is likely above the robot
    // to avoid dividing by 0
    Angle face_ball_orientation;
    if ((ball.position() - robot_position).length() == 0)
    {
        face_ball_orientation = event.common.robot.orientation();
    }
    else
    {
        face_ball_orientation = (ball.position() - robot_position).orientation();
    }

    // backup by the length of the ball
    Point backup_position =
        robot_position + ball.velocity().normalize(ROBOT_MAX_RADIUS_METERS);
    event.common.set_primitive(std::make_shared<MovePrimitive>(
        event.common.robot, backup_position, face_ball_orientation,
        TbotsProto::MaxAllowedSpeedMode::PHYSICAL_LIMIT,
        TbotsProto::ObstacleAvoidanceMode::AGGRESSIVE,
        TbotsProto::DribblerMode::MAX_FORCE, TbotsProto::BallCollisionType::ALLOW,
        AutoChipOrKick{AutoChipOrKickMode::OFF, 0}));
}

bool PassDefenderFSM::ballNearbyWithoutThreat(const Update& event)
{
    Point robot_position   = event.common.robot.position();
    double ball_position_x = event.common.world_ptr->ball().position().x();
    std::optional<Robot> nearest_enemy =
            event.common.world_ptr->enemyTeam().getNearestRobot(robot_position);
    if (nearest_enemy)
    {
        // Get the ball if ball is closer to robot than enemy threat by threshold ratio
        double ball_distance =
                distance(robot_position, event.common.world_ptr->ball().position());
        double nearest_enemy_distance =
                distance(robot_position, nearest_enemy->position());

        return ball_distance < nearest_enemy_distance * MAX_GET_BALL_RATIO_THRESHOLD &&
               ball_position_x < 0 && ball_distance <= MAX_GET_BALL_RADIUS_M &&
               event.common.world_ptr->ball().velocity().length() <=
               MAX_BALL_SPEED_TO_GET_MS;
    }
    else
    {
        return true;
    }
}

void PassDefenderFSM::prepareGetPossession(
        const Update& event, boost::sml::back::process<DribbleFSM::Update> processEvent)
{
    Point ball_position       = event.common.world_ptr->ball().position();
    Point enemy_goal_center   = event.common.world_ptr->field().enemyGoal().centre();
    Vector ball_to_net_vector = Vector(enemy_goal_center.x() - ball_position.x(),
                                       enemy_goal_center.y() - ball_position.y());
    DribbleFSM::ControlParams control_params{
            .dribble_destination       = ball_position,
            .final_dribble_orientation = ball_to_net_vector.orientation(),
            .allow_excessive_dribbling = false};
    processEvent(DribbleFSM::Update(control_params, event.common));
}