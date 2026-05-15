#include "software/sensor_fusion/filter/robot_filter.h"

#include <gtest/gtest.h>

#include "software/test_util/equal_within_tolerance.h"

TEST(RobotFilterTest, no_match_robot_data_robot_state_expired_test)
{
    Robot robot(1, Point(0, 0), Vector(0, 0), Angle::fromRadians(0),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    RobotFilter robot_filter(robot, Duration::fromSeconds(10));
    // Provide a detection for a DIFFERENT robot (id 2) at time 11.
    // The filter robot is ID 1. At time 11, robot 1 has had no data for 11 seconds.
    // Expiry is 10s. So it should expire.
    std::vector<RobotDetection> new_robot_data = {
        {2, Point(2, 0), Angle::fromRadians(1), 0.5, Timestamp::fromSeconds(11)}};
    EXPECT_EQ(std::nullopt, robot_filter.getFilteredData(new_robot_data));
}

TEST(RobotFilterTest, no_match_robot_data_robot_state_not_expired_test)
{
    Robot robot(1, Point(0, 0), Vector(0, 0), Angle::fromRadians(0),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    RobotFilter robot_filter(robot, Duration::fromSeconds(10));
    std::vector<RobotDetection> new_robot_data = {
        {2, Point(2, 0), Angle::fromRadians(1), 0.5, Timestamp::fromSeconds(9)}};
    
    // No data for robot 1, but time is 9s which is < 10s expiry.
    // Should return the original state (time 0).
    auto filtered_robot = robot_filter.getFilteredData(new_robot_data);
    ASSERT_TRUE(filtered_robot.has_value());
    EXPECT_EQ(robot.id(), filtered_robot->id());
    EXPECT_EQ(robot.position(), filtered_robot->position());
    EXPECT_EQ(robot.velocity(), filtered_robot->velocity());
}

TEST(RobotFilterTest, stationary_robot_test)
{
    Robot robot(1, Point(1.0, 1.0), Vector(0, 0), Angle::fromRadians(0.5),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    RobotFilter robot_filter(robot, Duration::fromSeconds(10));

    // Feed several stationary measurements
    for (int i = 1; i <= 10; ++i)
    {
        std::vector<RobotDetection> new_robot_data = {
            {1, Point(1.0, 1.0), Angle::fromRadians(0.5), 1.0, Timestamp::fromSeconds(i * 0.1)}};
        robot_filter.getFilteredData(new_robot_data);
    }

    auto filtered_robot = robot_filter.getFilteredData({});
    ASSERT_TRUE(filtered_robot.has_value());
    
    EXPECT_TRUE(TestUtil::equalWithinTolerance(1.0, filtered_robot->position().x(), 0.1));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(1.0, filtered_robot->position().y(), 0.1));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(0.0, filtered_robot->velocity().x(), 0.1));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(0.0, filtered_robot->velocity().y(), 0.1));
}

TEST(RobotFilterTest, horizon_buffer_out_of_order_test)
{
    Robot robot(1, Point(0.0, 0.0), Vector(0, 0), Angle::fromRadians(0.0),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    
    // Filter 1: In-order packets
    RobotFilter filter_in_order(robot, Duration::fromSeconds(10));
    filter_in_order.getFilteredData({{1, Point(1.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.01)}});
    filter_in_order.getFilteredData({{1, Point(2.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.02)}});
    filter_in_order.getFilteredData({{1, Point(3.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.03)}});
    auto robot_in_order = filter_in_order.getFilteredData({});

    // Filter 2: Out-of-order packets (t=0.03 arrives before t=0.02)
    RobotFilter filter_out_of_order(robot, Duration::fromSeconds(10));
    filter_out_of_order.getFilteredData({{1, Point(1.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.01)}});
    filter_out_of_order.getFilteredData({{1, Point(3.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.03)}});
    filter_out_of_order.getFilteredData({{1, Point(2.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.02)}});
    auto robot_out_of_order = filter_out_of_order.getFilteredData({});

    ASSERT_TRUE(robot_in_order.has_value());
    ASSERT_TRUE(robot_out_of_order.has_value());

    // Because the horizon buffer rewinds and replays, the end state should be perfectly identical
    EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_in_order->position().x(), robot_out_of_order->position().x(), 1e-4));
    EXPECT_TRUE(TestUtil::equalWithinTolerance(robot_in_order->velocity().x(), robot_out_of_order->velocity().x(), 1e-4));
}

TEST(RobotFilterTest, horizon_buffer_discard_old_test)
{
    Robot robot(1, Point(0.0, 0.0), Vector(0, 0), Angle::fromRadians(0.0),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    RobotFilter robot_filter(robot, Duration::fromSeconds(10));

    robot_filter.getFilteredData({{1, Point(1.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.05)}});
    robot_filter.getFilteredData({{1, Point(2.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.20)}}); // Advances time to 0.20
    
    // This packet is at 0.01. The cutoff is 0.20 - 0.100 = 0.10. 
    // It should be completely discarded and have no effect.
    robot_filter.getFilteredData({{1, Point(100.0, 0), Angle::fromRadians(0), 1.0, Timestamp::fromSeconds(0.01)}});
    
    auto final_robot = robot_filter.getFilteredData({});
    ASSERT_TRUE(final_robot.has_value());
    
    // Position should NOT be influenced by the 100.0 outlier at 0.01
    EXPECT_TRUE(final_robot->position().x() < 10.0); 
}

TEST(RobotFilterTest, angle_wrapping_test)
{
    Robot robot(1, Point(0, 0), Vector(0, 0), Angle::fromDegrees(170.0),
                AngularVelocity::fromRadians(0), Timestamp::fromSeconds(0));
    RobotFilter robot_filter(robot, Duration::fromSeconds(10));

    // Robot rotates from 170 to 175 to -175 (which is 185)
    robot_filter.getFilteredData({{1, Point(0, 0), Angle::fromDegrees(175), 1.0, Timestamp::fromSeconds(0.1)}});
    robot_filter.getFilteredData({{1, Point(0, 0), Angle::fromDegrees(-175), 1.0, Timestamp::fromSeconds(0.2)}});

    auto filtered_robot = robot_filter.getFilteredData({});
    ASSERT_TRUE(filtered_robot.has_value());

    // Angular velocity should be positive (rotating counter-clockwise)
    // Distance from 170 to -175 is 15 degrees over 0.2 seconds -> ~75 deg/sec
    EXPECT_TRUE(filtered_robot->angularVelocity().toDegrees() > 0.0);
    EXPECT_TRUE(TestUtil::equalWithinTolerance(-175.0, filtered_robot->orientation().toDegrees(), 10.0));
}