import gzip
import os


# Feature 1: Extract kinematics dataset - corner cases
def test_f1_corner_multiple_replay_chunks(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    for i in range(3):
        with gzip.open(log_dir / f"{i}.replay", "wb"):
            pass
    res = run_binary("extract_kinematics_events", [str(log_dir)], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "Extraction complete" in res.stdout


def test_f1_corner_non_replay_files_ignored(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    (log_dir / "notes.txt").write_text("not a replay file")
    res = run_binary("extract_kinematics_events", [str(log_dir)])
    assert res.returncode != 0
    assert "No replay files found" in res.stderr


def test_f1_corner_corrupted_replay_file(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    (log_dir / "0.replay").write_bytes(b"not a gzip file")
    res = run_binary("extract_kinematics_events", [str(log_dir)])
    assert res.returncode != 0


def test_f1_corner_extra_args_ignored(dummy_log_dir, run_binary, tmp_path):
    res = run_binary(
        "extract_kinematics_events", [dummy_log_dir, "extra_arg"], cwd=str(tmp_path)
    )
    assert res.returncode == 0


def test_f1_corner_trailing_slash_log_dir(dummy_log_dir, run_binary, tmp_path):
    res = run_binary(
        "extract_kinematics_events", [dummy_log_dir + os.sep], cwd=str(tmp_path)
    )
    assert res.returncode == 0


# Feature 2: Extract game events - corner cases
def test_f2_corner_multiple_replay_chunks(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    for i in range(3):
        with gzip.open(log_dir / f"{i}.replay", "wb"):
            pass
    res = run_binary("extract_game_events", [str(log_dir)], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "Extraction complete" in res.stdout


def test_f2_corner_non_replay_files_ignored(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    (log_dir / "notes.txt").write_text("not a replay file")
    res = run_binary("extract_game_events", [str(log_dir)])
    assert res.returncode != 0
    assert "No replay files found" in res.stderr


def test_f2_corner_corrupted_replay_file(run_binary, tmp_path):
    log_dir = tmp_path / "logs"
    log_dir.mkdir()
    (log_dir / "0.replay").write_bytes(b"not a gzip file")
    res = run_binary("extract_game_events", [str(log_dir)])
    assert res.returncode != 0


# Feature 3: Optuna physics tuner - corner cases
def test_f3_corner_db_is_directory(run_binary, tmp_path):
    (tmp_path / "physics_tuner.db").mkdir()
    res = run_binary("optuna_physics_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f3_corner_negative_trials_fresh_db(run_binary, tmp_path):
    res = run_binary("optuna_physics_tuner", ["--n_trials", "-1"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f3_corner_non_integer_trials(run_binary, tmp_path):
    res = run_binary(
        "optuna_physics_tuner", ["--n_trials", "abc"], cwd=str(tmp_path)
    )
    assert res.returncode == 2


def test_f3_corner_nonexistent_logs_dir(run_binary, tmp_path):
    res = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", "/nonexistent_dir_xyz", "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert res.returncode == 0


def test_f3_corner_unexpected_positional_arg(run_binary, tmp_path):
    res = run_binary("optuna_physics_tuner", ["positional_arg"], cwd=str(tmp_path))
    assert res.returncode == 2


# Feature 4: Optuna gameplay tuner - corner cases
def test_f4_corner_db_is_directory(run_binary, tmp_path):
    (tmp_path / "gameplay_tuner.db").mkdir()
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f4_corner_negative_trials_fresh_db(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "-1"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f4_corner_non_integer_trials(run_binary, tmp_path):
    res = run_binary(
        "optuna_gameplay_tuner", ["--n_trials", "abc"], cwd=str(tmp_path)
    )
    assert res.returncode == 2


def test_f4_corner_unexpected_positional_arg(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["positional_arg"], cwd=str(tmp_path))
    assert res.returncode == 2


# Feature 5: Unified CLI - corner cases
def test_f5_corner_pipeline_missing_value(run_binary):
    res = run_binary("auto_tune", ["--pipeline"])
    assert res.returncode == 2


def test_f5_corner_n_trials_non_integer(run_binary):
    res = run_binary("auto_tune", ["--n_trials", "abc"])
    assert res.returncode == 2


def test_f5_corner_unexpected_positional_arg(run_binary):
    res = run_binary("auto_tune", ["positional_arg"])
    assert res.returncode == 2


def test_f5_corner_sim2real_with_nonexistent_logs_dir(run_binary, tmp_path):
    res = run_binary(
        "auto_tune",
        ["--pipeline", "sim2real", "--logs_dir", "/nonexistent_dir_xyz", "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert "Starting sim2real physics tuning pipeline" in res.stdout


# Feature 6: Pausable execution (Optuna SQLite resume) - corner cases
def test_f6_corner_gameplay_tuner_corrupt_db(run_binary, tmp_path):
    (tmp_path / "gameplay_tuner.db").write_text("not a sqlite database")
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f6_corner_physics_tuner_accumulates_trials_across_runs(
    dummy_log_dir, run_binary, tmp_path
):
    res1 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    res2 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "2"],
        cwd=str(tmp_path),
    )
    assert res1.returncode == 0
    assert res2.returncode == 0
    assert "Using an existing study" in res2.stderr


# Feature 7: Bazel and runfiles integration - corner cases
def test_f7_corner_log_dir_with_spaces(run_binary, tmp_path):
    log_dir = tmp_path / "log dir with spaces"
    log_dir.mkdir()
    with gzip.open(log_dir / "0.replay", "wb"):
        pass
    res = run_binary("extract_kinematics_events", [str(log_dir)], cwd=str(tmp_path))
    assert res.returncode == 0


def test_f7_corner_relative_log_dir_path(dummy_log_dir, run_binary, tmp_path):
    rel_path = os.path.relpath(dummy_log_dir, start=str(tmp_path))
    res = run_binary("extract_kinematics_events", [rel_path], cwd=str(tmp_path))
    assert res.returncode == 0


def test_f7_corner_symlinked_log_dir(dummy_log_dir, run_binary, tmp_path):
    symlink_dir = tmp_path / "symlinked_logs"
    os.symlink(dummy_log_dir, symlink_dir)
    res = run_binary("extract_kinematics_events", [str(symlink_dir)], cwd=str(tmp_path))
    assert res.returncode == 0


if __name__ == "__main__":
    import sys
    import pytest

    sys.exit(pytest.main(sys.argv))
