#include "software/sensor_fusion/filter/robot_team_filter.h"

#include <gtest/gtest.h>
#include <string.h>
#include "software/test_util/equal_within_tolerance.h"

TEST(RobotTeamFilterTest, one_robot_detection_update_test)
{
    Team old_team = Team(Duration::fromMilliseconds(1000));
    std::vector<RobotDetection> robot_detections;
    RobotDetection robot_detection;
    RobotTeamFilter robot_team_filter;

    // old team starts with a robot with id = 0
    old_team.updateRobots(
        {Robot(0, Point(0, 1), Vector(-1, -2), Angle::half(),
               AngularVelocity::threeQuarter(), Timestamp::fromSeconds(0))});

    robot_detection.id          = 0;
    robot_detection.position    = Point(1.0, -2.5);
    robot_detection.orientation = Angle::fromRadians(0.5);
    robot_detection.confidence  = 1.0;
    robot_detection.timestamp   = Timestamp::fromSeconds(1);
    robot_detections.push_back(robot_detection);

    // robot detection also has id = 0
    Team new_team = robot_team_filter.getFilteredData(old_team, robot_detections);

    auto robots = new_team.getAllRobots();

    EXPECT_EQ(1, robots.size());
    EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detection.position.x(), robots[0].currentState().position().x(), 1e-3));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detection.position.y(), robots[0].currentState().position().y(), 1e-3));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detection.orientation.toRadians(), robots[0].currentState().orientation().toRadians(), 1e-3));
    EXPECT_EQ(robot_detection.timestamp, robots[0].timestamp());
}

TEST(RobotTeamFilterTest, detections_with_same_timestamp_test)
{
    Team old_team = Team(Duration::fromMilliseconds(1000));
    std::vector<RobotDetection> robot_detections;
    RobotDetection robot_detection;
    RobotTeamFilter robot_team_filter;

    unsigned int num_robots = 6;
    for (unsigned int i = 0; i < num_robots; i++)
    {
        robot_detection.id          = i;
        robot_detection.position    = Point(Vector(0.5, -0.25) * i);
        robot_detection.orientation = Angle::fromRadians(0.1 * i);
        robot_detection.confidence  = i / (static_cast<double>(num_robots));
        robot_detection.timestamp   = Timestamp::fromSeconds(.5);
        robot_detections.push_back(robot_detection);
    }

    Team new_team = robot_team_filter.getFilteredData(old_team, robot_detections);

    EXPECT_EQ(num_robots, new_team.numRobots());

    for (unsigned int i = 0; i < num_robots; i++)
    {
        EXPECT_NE(std::nullopt, new_team.getRobotById(i));
        Robot robot = *new_team.getRobotById(i);
        EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detections[i].position.x(), robot.currentState().position().x(), 1e-3));
        EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detections[i].position.y(), robot.currentState().position().y(), 1e-3));
        EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_detections[i].orientation.toRadians(), robot.currentState().orientation().toRadians(), 1e-3));
        EXPECT_EQ(robot_detections[i].timestamp, robot.timestamp());
    }
}

TEST(RobotTeamFilterTest, detections_with_different_times_test)
{
    Team old_team = Team(Duration::fromMilliseconds(1000));
    std::vector<RobotDetection> robot_detections;
    RobotDetection robot_detection;
    RobotTeamFilter robot_team_filter;

    unsigned int num_robots = 6;
    for (unsigned int i = 0; i < num_robots; i++)
    {
        robot_detection.id          = i;
        robot_detection.position    = Point(Vector(0.5, -0.25) * i);
        robot_detection.orientation = Angle::fromRadians(0.1 * i);
        robot_detection.confidence  = i / (static_cast<double>(num_robots));
        // use different timestamps for each detection
        robot_detection.timestamp = Timestamp::fromSeconds(i);
        robot_detections.push_back(robot_detection);
    }

    Team new_team = robot_team_filter.getFilteredData(old_team, robot_detections);

    // After 6 seconds, robots with timestamps < 5s are expired because duration = 1s.
    // The timestamp for i=5 is 5. Timestamps 0,1,2,3 are removed. Timestamp 4 is kept since 5 - 4 <= 1s.
    EXPECT_EQ(2, new_team.numRobots());
}
