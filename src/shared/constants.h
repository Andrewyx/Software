#pragma once
#include <math.h>

// Some platformio targets don't support STL, so we can't
// use unordered_map, string, .... We guard all networking stuff with
#ifndef PLATFORMIO_BUILD
#include <string>
#include <unordered_map>

// Networking
// the IPv6 multicast address, only ff02 is important, the rest is random
// see https://en.wikipedia.org/wiki/Solicited-node_multicast_address for why ff02 matters
static const std::unordered_map<int, std::string> ROBOT_MULTICAST_CHANNELS = {
    {0, "ff02::c3d0:42d2:bb00"},  {1, "ff02::c3d0:42d2:bb01"},
    {2, "ff02::c3d0:42d2:bb02"},  {3, "ff02::c3d0:42d2:bb03"},
    {4, "ff02::c3d0:42d2:bb04"},  {5, "ff02::c3d0:42d2:bb05"},
    {6, "ff02::c3d0:42d2:bb06"},  {7, "ff02::c3d0:42d2:bb07"},
    {8, "ff02::c3d0:42d2:bb08"},  {9, "ff02::c3d0:42d2:bb08"},
    {10, "ff02::c3d0:42d2:bb10"}, {11, "ff02::c3d0:42d2:bb11"},
    {12, "ff02::c3d0:42d2:bb12"}, {13, "ff02::c3d0:42d2:bb13"},
    {14, "ff02::c3d0:42d2:bb14"}, {15, "ff02::c3d0:42d2:bb15"}};

// PlotJuggler's default host and port
// Should be updated to your local machine's IP address if
// you want to plot from the robot
static const std::string PLOTJUGGLER_GUI_DEFAULT_HOST        = "127.0.0.1";
static const short unsigned int PLOTJUGGLER_GUI_DEFAULT_PORT = 9870;

// ProtoLogger constants for replay files
static const std::string REPLAY_FILE_EXTENSION      = "replay";
static const std::string REPLAY_METADATA_DELIMITER  = ",";
static const std::string REPLAY_FILE_VERSION_PREFIX = "version:";
static const unsigned int REPLAY_FILE_VERSION       = 2;

#endif  // PLATFORMIO_BUILD

// Redis default server connections properties
#define REDIS_HOST_LENGTH 10
static const char REDIS_DEFAULT_HOST[REDIS_HOST_LENGTH] = "127.0.0.1";
static const short unsigned int REDIS_DEFAULT_PORT      = 6379;

// the UDP port robots are listening to for primitives
static const short unsigned int PRIMITIVE_PORT = 42070;

// the port the AI receives msgs from the robot
static constexpr short unsigned int ROBOT_STATUS_PORT                         = 42071;
static constexpr short unsigned int ROBOT_LOGS_PORT                           = 42072;
static constexpr short unsigned int ROBOT_CRASH_PORT                          = 42074;
static constexpr short unsigned int NETWORK_COMM_TEST_PORT                    = 42075;
static constexpr short unsigned int ROBOT_TO_FULL_SYSTEM_IP_NOTIFICATION_PORT = 42073;
static constexpr short unsigned int FULL_SYSTEM_TO_ROBOT_IP_NOTIFICATION_PORT = 42076;

// maximum transfer unit of the network interface
// this is an int to avoid Wconversion with lwip
static const short unsigned int MAXIMUM_TRANSFER_UNIT_BYTES = 1500;

// This file contains all constants that are shared between our software (AI)
// and firmware code. Since this needs to be compiled by both C and C++, everything
// should be defined in a way that's compatible with C.

/* Game Rules */
// The max allowed speed of the ball, in metres per second
// https://robocup-ssl.github.io/ssl-rules/sslrules.html#_ball_speed
static const double BALL_MAX_SPEED_METERS_PER_SECOND = 6.5;
// The safe max speed of the ball that we should shoot at, in metres per second
static const double BALL_SAFE_MAX_SPEED_METERS_PER_SECOND =
    BALL_MAX_SPEED_METERS_PER_SECOND - 0.5;
// The distance that the ball has to travel for it to be considered in play
// after a kick-off, free kick, or penalty kick.
// https://robocup-ssl.github.io/ssl-rules/sslrules.html#_ball_in_and_out_of_play
static const double BALL_IN_PLAY_DISTANCE_THRESHOLD_METERS = 0.05;
// The max allowed height of the robots, in metres
static const double ROBOT_MAX_HEIGHT_METERS = 0.15;
// The max allowed radius of the robots, in metres
static const double ROBOT_MAX_RADIUS_METERS = 0.09;
// The distance from the center of the robot to the front face (the flat part), in meters
static const double DIST_TO_FRONT_OF_ROBOT_METERS = 0.078;
// The approximate radius of the ball according to the SSL rulebook
static const double BALL_MAX_RADIUS_METERS = 0.0215;
// According to the rules, 80% of the ball must be seen at all times. Robots may not
// cover more than 20% of the ball
static const double MAX_FRACTION_OF_BALL_COVERED_BY_ROBOT = 0.2;

// The radius of a circle region where ball placement is acceptable (in meters).
constexpr double BALL_PLACEMENT_TOLERANCE_RADIUS_METERS = 0.15;
// The radius of the outer region where robots are not allowed to be during ball
// placement (in meters)
constexpr double BALL_PLACEMENT_ROBOT_AVOID_RADIUS_METERS = 0.5;
// The time limit for ball placement in seconds
constexpr int BALL_PLACEMENT_TIME_LIMIT_S = 30;

// The mass of a standard golf ball, as defined by https://en.wikipedia.org/wiki/Golf_ball
constexpr double BALL_MASS_KG = 0.004593;
// The max allowed speed of the robot when the stop command is issued, in meters per
// second
static const double STOP_COMMAND_ROBOT_MAX_SPEED_METERS_PER_SECOND = 1.5;
// The margin by which the speed of the robot should stay below the maximum allowed speed
// when the stop command is issued
static const double STOP_COMMAND_SPEED_SAFETY_MARGIN_METERS_PER_SECOND = 0.2;
// The max allowed speed of the robot before collisions would incur a foul
static const double COLLISION_ALLOWED_ROBOT_MAX_SPEED_METERS_PER_SECOND = 0.5;
// The minimum distance from the ball all robots must be when the stop command is issued
// https://robocup-ssl.github.io/ssl-rules/sslrules.html#_stop
static const double STOP_COMMAND_BALL_AVOIDANCE_DISTANCE_M = 0.5;
// The maximum speed attainable by enemy robots
static const double ENEMY_ROBOT_MAX_SPEED_METERS_PER_SECOND = 3.0;
// The maximum acceleration achievable by enemy robots, in metres per seconds squared.
static const double ENEMY_ROBOT_MAX_ACCELERATION_METERS_PER_SECOND_SQUARED = 4.0;

static const double ACCELERATION_DUE_TO_GRAVITY_METERS_PER_SECOND_SQUARED = 9.81;
// The minimum distance the non-placing team must keep to avoid ball placement
// interference
static const double ENEMY_BALL_PLACEMENT_DISTANCE_METERS = 0.5;

/*Simulator Constants*/

// these values are set in coordination with other objects the ball will collide with.
// the resulting coefficient of friction is the product of both objects friction value.

// the sliding friction the ball experiences in Newtons
constexpr double BALL_SLIDING_FRICTION_NEWTONS = 1.0;
// the restitution of the ball
constexpr double BALL_RESTITUTION = 1.0;
// the transition factor where the ball goes from sliding to rolling
constexpr double FRICTION_TRANSITION_FACTOR = 5.0 / 7.0;
// the stationary ball speed in m/s
constexpr double STATIONARY_BALL_SPEED_METERS_PER_SECOND = 0.01;
// the acceleration the ball experiences due to rolling friction in m/s^2
constexpr double BALL_ROLLING_FRICTION_DECELERATION_METERS_PER_SECOND_SQUARED = -0.5;
// the acceleration the ball experiences due to sliding friction in m/s^2
constexpr double BALL_SLIDING_FRICTION_DECELERATION_METERS_PER_SECOND_SQUARED =
    -BALL_SLIDING_FRICTION_NEWTONS / BALL_MASS_KG;

/* Unit Conversion */
static const double MILLIMETERS_PER_METER = 1000.0;
static const double METERS_PER_MILLIMETER = 1.0 / 1000.0;
static const double CENTIMETERS_PER_METER = 100.0;
static const double METERS_PER_CENTIMETER = 1.0 / 100.0;

static const double CENTIRADIANS_PER_RADIAN = 100.0;
static const double RADIANS_PER_CENTIRADIAN = 1.0 / 100.0;

static const double NANOSECONDS_PER_MILLISECOND  = 1000000.0;
static const double NANOSECONDS_PER_SECOND       = 1000000000.0;
static const double MICROSECONDS_PER_MILLISECOND = 1000.0;
static const double MICROSECONDS_PER_SECOND      = 1000000.0;
static const double MILLISECONDS_PER_SECOND      = 1000.0;
static const double SECONDS_PER_MICROSECOND      = 1.0 / 1000000.0;
static const double SECONDS_PER_NANOSECOND       = 1.0 / 1000000000.0;
static const double SECONDS_PER_MILLISECOND      = 1.0 / 1000.0;
static const double MILLISECONDS_PER_MICROSECOND = 1.0 / 1000.0;
static const double MILLISECONDS_PER_NANOSECOND  = 1.0 / 1000000.0;
static const double SECONDS_PER_MINUTE           = 60.0;

static const double DEFAULT_SIMULATOR_TICK_RATE_SECONDS_PER_TICK =
    1.0 / 60.0;  // corresponds to 60 Hz
static const double DEFAULT_SIMULATOR_TICK_RATE_MILLISECONDS_PER_TICK =
    DEFAULT_SIMULATOR_TICK_RATE_SECONDS_PER_TICK * 1000;

// The total number of robot ids on one team
static const unsigned int MAX_ROBOT_IDS_PER_SIDE = 8;
// The total number of possible robot ids between two teams
static const unsigned int MAX_ROBOT_IDS = MAX_ROBOT_IDS_PER_SIDE * 2;

// How many robots are allowed in each division
static const unsigned int DIV_A_NUM_ROBOTS = 11;
static const unsigned int DIV_B_NUM_ROBOTS = 6;

// The maximum time in seconds given to Full System to cleanly exit the process.
static const double MAX_TIME_TO_EXIT_FULL_SYSTEM_SEC = 0.5;

// Battery Constants
static const unsigned NUM_CELLS_IN_BATTERY    = 3;
static const unsigned NUM_BATTERIES_IN_SERIES = 2;
static const double MAX_SINGLE_CELL_VOLTAGE   = 4.2;
static const double MIN_SINGLE_CELL_VOLTAGE   = 3.5 + 0.1;  // +0.1v headroom

static const double MIN_BATTERY_VOLTAGE =
    MIN_SINGLE_CELL_VOLTAGE * NUM_CELLS_IN_BATTERY * NUM_BATTERIES_IN_SERIES;
static const double MAX_BATTERY_VOLTAGE =
    MAX_SINGLE_CELL_VOLTAGE * NUM_CELLS_IN_BATTERY * NUM_BATTERIES_IN_SERIES;
static const double BATTERY_WARNING_VOLTAGE = MIN_BATTERY_VOLTAGE + 1.0;  // 1V headroom

// Chick Capacitor Constants
static const double MIN_CAPACITOR_VOLTAGE = 0;
static const double MAX_CAPACITOR_VOLTAGE = 250.0 + 50.0;  // +50v headroom

static const unsigned int ROBOT_CHIP_ANGLE_DEGREES = 45;
static const double CHICKER_TIMEOUT                = 3 * MILLISECONDS_PER_SECOND;

// Kick Spd to Pulse Width Safety Constraint Constants

static const int MAX_KICK_CONSTANT       = 850;
static const double MAX_KICK_COEFFICIENT = 0.4;

// Arduino

// UART baud rate used to communicate between system and arudino
static const long ARDUINO_BAUD_RATE = 115200;

/*
 * each estop message is one byte and is defined as follows
 * bit 0 (least significant bit): estop state, a value of 1 is play, 0 is stop
 * bit 1-7: set to 0
 * any other message received is considered a EstopState::STATUS_ERROR
 */
static const int ESTOP_MESSAGE_SIZE_BYTES = 1;

static const unsigned char ESTOP_PLAY_MSG = 0;
static const unsigned char ESTOP_STOP_MSG = 1;

// Number of times to send a STOP primitive when robot is disconnected from Manual Control
static const unsigned int NUM_TIMES_SEND_STOP = 10;
// How long a robot should receive no RobotStatus messages for until it is considered
// disconnected
static const double DISCONNECT_DURATION_MS = 1 * MILLISECONDS_PER_SECOND;

// product and vendor id for Arduino Uno Rev3 (retrieved from
// http://www.linux-usb.org/usb.ids )
#define ARDUINO_ID_LENGTH 5
static const char ARDUINO_VENDOR_ID[ARDUINO_ID_LENGTH]  = "2341";
static const char ARDUINO_PRODUCT_ID[ARDUINO_ID_LENGTH] = "0043";

// Number of times thunderloop should tick per second
static const unsigned THUNDERLOOP_HZ = 300u;

static const unsigned NUM_GENEVA_ANGLES = 5;

// Jetson Nano Constants
static const double MAX_JETSON_TEMP_C = 97;
