import os
import sys
import math
import pandas as pd

from software.thunderscope.replay.proto_player import ProtoPlayer

def distance(p1_x, p1_y, p2_x, p2_y):
    return math.hypot(p1_x - p2_x, p1_y - p2_y)

def main():
    if len(sys.argv) < 2:
        print("Usage: extract_game_events.py <log_folder_path>")
        sys.exit(1)
        
    log_folder_path = sys.argv[1]
    
    sorted_chunks = ProtoPlayer.sort_and_get_replay_files(log_folder_path)
    if not sorted_chunks:
        print("No replay files found.")
        return
        
    version = ProtoPlayer.get_replay_chunk_format_version(sorted_chunks[0])
    
    pass_data = []
    interception_data = []
    
    possession = None
    POSSESSION_THRESHOLD = 0.15 # meters
    
    for chunk_path in sorted_chunks:
        chunk_data = ProtoPlayer.load_replay_chunk(chunk_path, version)
        for log_entry in chunk_data:
            try:
                timestamp, proto_class, proto = ProtoPlayer.unpack_log_entry(log_entry, version)
            except Exception:
                continue
                
            proto_name = proto_class.__name__
            
            if proto_name == 'Pass':
                try:
                    row = {
                        'timestamp': timestamp,
                        'passer_x': proto.passer_point.x_meters,
                        'passer_y': proto.passer_point.y_meters,
                        'receiver_x': proto.receiver_point.x_meters,
                        'receiver_y': proto.receiver_point.y_meters,
                        'speed': proto.pass_speed_m_per_s
                    }
                    pass_data.append(row)
                except Exception:
                    pass
            elif proto_name == 'World':
                try:
                    bx = proto.ball.current_state.global_position.x_meters
                    by = proto.ball.current_state.global_position.y_meters
                except Exception:
                    continue
                
                min_f_dist = float('inf')
                for r in proto.friendly_team.team_robots:
                    try:
                        rx = r.current_state.global_position.x_meters
                        ry = r.current_state.global_position.y_meters
                        d = distance(bx, by, rx, ry)
                        if d < min_f_dist:
                            min_f_dist = d
                    except Exception:
                        pass
                        
                min_e_dist = float('inf')
                for r in proto.enemy_team.team_robots:
                    try:
                        rx = r.current_state.global_position.x_meters
                        ry = r.current_state.global_position.y_meters
                        d = distance(bx, by, rx, ry)
                        if d < min_e_dist:
                            min_e_dist = d
                    except Exception:
                        pass
                        
                new_possession = possession
                if min_f_dist < POSSESSION_THRESHOLD:
                    new_possession = 'Friendly'
                elif min_e_dist < POSSESSION_THRESHOLD:
                    new_possession = 'Enemy'
                    
                if possession == 'Friendly' and new_possession == 'Enemy':
                    interception_data.append({'timestamp': timestamp, 'type': 'Interception'})
                    
                if new_possession is not None:
                    possession = new_possession

    workspace_dir = os.environ.get('BUILD_WORKSPACE_DIRECTORY', '.')
    output_dir = os.path.join(workspace_dir, "software/sim_to_real")
    os.makedirs(output_dir, exist_ok=True)
    
    if pass_data:
        pd.DataFrame(pass_data).to_csv(os.path.join(output_dir, "game_passes.csv"), index=False)
    else:
        pd.DataFrame(columns=["timestamp"]).to_csv(os.path.join(output_dir, "game_passes.csv"), index=False)

    if interception_data:
        pd.DataFrame(interception_data).to_csv(os.path.join(output_dir, "game_interceptions.csv"), index=False)
    else:
        pd.DataFrame(columns=["timestamp"]).to_csv(os.path.join(output_dir, "game_interceptions.csv"), index=False)
    
    print(f"Extraction complete. Passes: {len(pass_data)} rows, Interceptions: {len(interception_data)} rows.")

if __name__ == "__main__":
    main()

