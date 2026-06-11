import os
import sys
import pandas as pd
from google.protobuf.json_format import MessageToDict

from software.thunderscope.replay.proto_player import ProtoPlayer

def main():
    if len(sys.argv) < 2:
        print("Usage: extract_kinematics_events.py <log_folder_path>")
        sys.exit(1)
        
    log_folder_path = sys.argv[1]
    
    sorted_chunks = ProtoPlayer.sort_and_get_replay_files(log_folder_path)
    if not sorted_chunks:
        print("No replay files found.")
        return
        
    version = ProtoPlayer.get_replay_chunk_format_version(sorted_chunks[0])
    
    world_data = []
    intent_data = []
    seen_protos = set()
    
    for chunk_path in sorted_chunks:
        chunk_data = ProtoPlayer.load_replay_chunk(chunk_path, version)
        for log_entry in chunk_data:
            try:
                timestamp, proto_class, proto = ProtoPlayer.unpack_log_entry(log_entry, version)
            except Exception:
                continue
                
            proto_name = proto_class.__name__
            seen_protos.add(proto_name)
            
            if proto_name == 'World':
                row = {'timestamp': timestamp}
                try:
                    row['ball_x'] = proto.ball.current_state.global_position.x_meters
                    row['ball_y'] = proto.ball.current_state.global_position.y_meters
                except Exception:
                    pass
                
                for r in proto.friendly_team.team_robots:
                    try:
                        row[f'friendly_{r.id}_x'] = r.current_state.global_position.x_meters
                        row[f'friendly_{r.id}_y'] = r.current_state.global_position.y_meters
                    except Exception:
                        pass
                
                for r in proto.enemy_team.team_robots:
                    try:
                        row[f'enemy_{r.id}_x'] = r.current_state.global_position.x_meters
                        row[f'enemy_{r.id}_y'] = r.current_state.global_position.y_meters
                    except Exception:
                        pass
                    
                world_data.append(row)
                
            elif proto_name in ('Intent', 'PlayInfo', 'Tactic'):
                row = {'timestamp': timestamp, 'type': proto_name}
                if proto_name == 'PlayInfo':
                    for robot_id, tactic in proto.robot_tactic_assignment.items():
                        try:
                            row[f'robot_{robot_id}_tactic'] = tactic.tactic_name
                            row[f'robot_{robot_id}_fsm'] = tactic.tactic_fsm_state
                        except Exception:
                            pass
                else:
                    try:
                        if proto_name == 'Tactic':
                            active_tactic = proto.WhichOneof('tactic')
                            row['active_tactic'] = active_tactic
                            if active_tactic:
                                payload = getattr(proto, active_tactic)
                                if active_tactic == 'move':
                                    row['destination_x'] = getattr(payload.destination, 'x', getattr(payload.destination, 'x_meters', 0.0))
                                    row['destination_y'] = getattr(payload.destination, 'y', getattr(payload.destination, 'y_meters', 0.0))
                                elif active_tactic == 'kick':
                                    row['kick_speed'] = getattr(payload, 'kick_speed_meters_per_second', 0.0)
                                payload_dict = MessageToDict(payload, preserving_proto_field_name=True)
                            else:
                                payload_dict = {}
                        else:
                            payload_dict = MessageToDict(proto, preserving_proto_field_name=True)
                            
                        for k, v in payload_dict.items():
                            if isinstance(v, dict):
                                for sub_k, sub_v in v.items():
                                    row[f"{k}.{sub_k}"] = str(sub_v)
                            else:
                                row[k] = str(v)
                    except Exception:
                        pass
                intent_data.append(row)

    print(f"Seen protos: {seen_protos}")

    workspace_dir = os.environ.get('BUILD_WORKSPACE_DIRECTORY', '.')
    output_dir = os.path.join(workspace_dir, "software/sim_to_real")
    os.makedirs(output_dir, exist_ok=True)

    if world_data:
        pd.DataFrame(world_data).to_csv(os.path.join(output_dir, "kinematics_world.csv"), index=False)
    else:
        pd.DataFrame(columns=["timestamp"]).to_csv(os.path.join(output_dir, "kinematics_world.csv"), index=False)

    if intent_data:
        pd.DataFrame(intent_data).to_csv(os.path.join(output_dir, "kinematics_intent.csv"), index=False)
    else:
        pd.DataFrame(columns=["timestamp"]).to_csv(os.path.join(output_dir, "kinematics_intent.csv"), index=False)
    
    print(f"Extraction complete. World: {len(world_data)} rows, Intent/PlayInfo: {len(intent_data)} rows.")

if __name__ == "__main__":
    main()
