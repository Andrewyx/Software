from rich import print
from rich.live import Live
from rich.table import Table
import software.python_bindings as tbots_cpp
from software.py_constants import *
from typer_shell import make_typer_shell
from google.protobuf.message import Message
from proto.import_all_protos import *
import redis

class RobotDiagnosticsCLI:
    def __init__(self) -> None:
        self.app = make_typer_shell(prompt="⚡ ")
        self.app.command(short_help="Rotates the robot")(self.rotate)
        self.app.command(short_help="Moves the robot")(self.move)
        self.app.command(short_help="Chips the chipper")(self.chip)
        self.app.command(short_help="Kicks the kicker")(self.kick)
        self.app.command(short_help="Show Robot stats")(self.stats)
        self.app.command(short_help="Spins the dribbler")(self.dribble)
        self.app.command(short_help="Restarts Thunderloop")(self.restart_thunderloop)
        self.app.command(short_help="Shows Redis Values")(self.redis)

        self.redis = redis.StrictRedis(
            host=REDIS_DEFAULT_HOST,
            port=REDIS_DEFAULT_PORT,
            charset="utf-8",
            decode_responses=True
        )
        self.channel_id = int(self.redis.get(ROBOT_ID_REDIS_KEY))

        # Receiver / probably want to fetch channel from redis cache
        self.receive_robot_status = tbots_cpp.RobotStatusProtoListener(
            str(getRobotMulticastChannel(self.channel_id)) + "%" + "eth0",
            ROBOT_STATUS_PORT,
            self.__receive_robot_status,
            True,
        )

        # Sender / What is the network interface here?
        self.send_primitive_set = tbots_cpp.PrimitiveSetProtoUdpSender(
            str(getRobotMulticastChannel(self.channel_id)) + "%" + "eth0", PRIMITIVE_PORT, True
        )

    def __receive_robot_status(self, robot_status: Message) -> None:
        """Forwards the given robot status to the full system along with the round-trip time

        :param robot_status: RobotStatus to forward to fullsystem
        """
        self.epoch_timestamp_seconds = robot_status.time_sent.epoch_timestamp_seconds
        self.battery_voltage = robot_status.power_status.battery_voltage
        self.robot_id = robot_status.robot_id
        self.primitive_packet_loss_percentage = robot_status.network_status.primitive_packet_loss_percentage
        self.running_primitive = robot_status.primitive_executor_status.running_primitive

    def __generate_stats_table(self) -> Table:
        """Make a new table with robot status information."""
        table = Table()
        table.add_column("Robot ID")
        table.add_column("Battery (V)")
        table.add_column("Packet Loss (%)")
        table.add_column("Status")
        table.add_column("Lifetime (s)")

        status = "[red]OFFLINE"
        if self.running_primitive:
            status = "[green]ONLINE"

        table.add_row(
            f"{self.redis.get(ROBOT_ID_REDIS_KEY)}",
            f"{self.battery_voltage:3.2f}",
            f"{self.primitive_packet_loss_percentage}",
            status,
            f"{self.epoch_timestamp_seconds}"
        )
        return table

    def __generate_redis_table(self) -> Table:
        """Make a new table with redis value information."""
        table = Table()
        table.add_column("Redis Value Name")
        table.add_column("Key")
        table.add_column("Value")

        table.add_row("Robot ID", f"{ROBOT_ID_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_ID_REDIS_KEY)}")
        table.add_row("Channel ID", f"{ROBOT_MULTICAST_CHANNEL_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_MULTICAST_CHANNEL_REDIS_KEY)}")
        table.add_row("Network Interface", f"{ROBOT_NETWORK_INTERFACE_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_NETWORK_INTERFACE_REDIS_KEY)}")
        table.add_row("Kick Constant", f"{ROBOT_KICK_CONSTANT_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_KICK_CONSTANT_REDIS_KEY)}")
        table.add_row("Kick Coefficient", f"{ROBOT_KICK_EXP_COEFF_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_KICK_EXP_COEFF_REDIS_KEY)}")
        table.add_row("Chip Pulse Width", f"{ROBOT_CHIP_PULSE_WIDTH_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_CHIP_PULSE_WIDTH_REDIS_KEY)}")
        table.add_row("Battery Voltage", f"{ROBOT_CURRENT_DRAW_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_CURRENT_DRAW_REDIS_KEY)}")
        table.add_row("Capacitor Voltage", f"{ROBOT_BATTERY_VOLTAGE_REDIS_KEY}",
                      f"{self.redis.get(ROBOT_BATTERY_VOLTAGE_REDIS_KEY)}")

        return table

    def __clamp(self, val: float, min_val: float, max_val: float) -> float:
        # Faster than numpy & fewer dependencies
        return min(max(val, min_val), max_val)
    def rotate(self, velocity_in_rad: float) -> None:
        # CLAMP SPEED
        MAX_SPEED_RAD = 4
        velocity_in_rad = self.__clamp(velocity_in_rad, -MAX_SPEED_RAD, MAX_SPEED_RAD)
        print(f"Rotating at {velocity_in_rad} rad/s")

    def move(self, direction: str, speed: float) -> None:
        default_commands: dict = {
            "forward": 90,
            "back": 270,
            "left": 180,
            "right": 0
        }
        direction = direction.strip()
        direction = direction.lower()
        # CLAMP SPEED
        MAX_VALUE = 100
        MIN_VALUE = 0
        speed = self.__clamp(speed, MIN_VALUE, MAX_VALUE)
        if direction in default_commands:
            print(f"Going {direction} and mapping to {default_commands[direction]} at the current speed {speed}")
        else:
            print("ERROR: INVALID COMMAND")

    def chip(self, distance_meter: float = 1.0) -> None:
        distance_meter = self.__clamp(distance_meter, 0, 2.0)
        print(f"Chipping {distance_meter} meters")

    def kick(self, speed_m_per_s: float = 2.0) -> None:
        speed_m_per_s = self.__clamp(speed_m_per_s, 0, 6.0)
        print(f"Kicking at {speed_m_per_s} meters per second")

    def stats(self) -> None:
        with Live(self.__generate_stats_table(), refresh_per_second=4) as live:
            while True:
                live.update(self.__generate_stats_table())

    def redis(self):
        with Live(self.__generate_redis_table(), refresh_per_second=4) as live:
            while True:
                live.update(self.__generate_redis_table())

    def dribble(self, velocity_rad_per_s: float) -> None:
        velocity_rad_per_s = self.__clamp(velocity_rad_per_s, 0, 5.0)
        print(f"Spinning dribbler at {velocity_rad_per_s} rad per second")

    def restart_thunderloop(self):
        # Execute some bash command
        pass

    def emote(self):
        pass

    def move_wheel(self):
        # py_inquire wheel choice or use sub shell
        pass


if __name__ == "__main__":
    while True:
        RobotDiagnosticsCLI().app()
