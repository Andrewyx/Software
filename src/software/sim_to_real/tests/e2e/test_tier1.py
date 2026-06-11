import os


# Feature 1: Extract kinematics dataset
def test_f1_extract_kinematics_basic(dummy_log_dir, run_binary, tmp_path):
    res = run_binary("extract_kinematics_events", [dummy_log_dir], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "Extraction complete" in res.stdout
    assert os.path.isfile(tmp_path / "software/sim_to_real/kinematics_world.csv")
    assert os.path.isfile(tmp_path / "software/sim_to_real/kinematics_intent.csv")


def test_f1_extract_kinematics_no_args(run_binary):
    res = run_binary("extract_kinematics_events", [])
    assert res.returncode != 0
    assert "Usage" in res.stdout


def test_f1_extract_kinematics_missing_log_dir(run_binary):
    res = run_binary("extract_kinematics_events", ["/nonexistent_dir_xyz"])
    assert res.returncode != 0


def test_f1_extract_kinematics_empty_log_dir(empty_log_dir, run_binary):
    res = run_binary("extract_kinematics_events", [empty_log_dir])
    assert res.returncode != 0
    assert "No replay files found" in res.stderr


def test_f1_extract_kinematics_log_dir_is_a_file(run_binary, tmp_path):
    log_file = tmp_path / "not_a_dir"
    log_file.write_text("not a directory")
    res = run_binary("extract_kinematics_events", [str(log_file)])
    assert res.returncode != 0


# Feature 2: Extract game events
def test_f2_extract_game_events_basic(dummy_log_dir, run_binary, tmp_path):
    res = run_binary("extract_game_events", [dummy_log_dir], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "Extraction complete" in res.stdout
    assert os.path.isfile(tmp_path / "software/sim_to_real/game_passes.csv")
    assert os.path.isfile(tmp_path / "software/sim_to_real/game_interceptions.csv")


def test_f2_extract_game_events_no_args(run_binary):
    res = run_binary("extract_game_events", [])
    assert res.returncode != 0
    assert "Usage" in res.stdout


def test_f2_extract_game_events_missing_log_dir(run_binary):
    res = run_binary("extract_game_events", ["/nonexistent_dir_xyz"])
    assert res.returncode != 0


def test_f2_extract_game_events_empty_log_dir(empty_log_dir, run_binary):
    res = run_binary("extract_game_events", [empty_log_dir])
    assert res.returncode != 0
    assert "No replay files found" in res.stderr


def test_f2_extract_game_events_log_dir_is_a_file(run_binary, tmp_path):
    log_file = tmp_path / "not_a_dir"
    log_file.write_text("not a directory")
    res = run_binary("extract_game_events", [str(log_file)])
    assert res.returncode != 0


# Feature 3: Optuna physics tuner
def test_f3_optuna_physics_tuner_help(run_binary):
    res = run_binary("optuna_physics_tuner", ["--help"])
    assert res.returncode == 0
    assert "--logs_dir" in res.stdout
    assert "--n_trials" in res.stdout


def test_f3_optuna_physics_tuner_basic(dummy_log_dir, run_binary, tmp_path):
    res = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert res.returncode == 0
    assert "robot_constants.h" in res.stdout
    assert os.path.isfile(tmp_path / "physics_tuner.db")


def test_f3_optuna_physics_tuner_default_logs_dir(run_binary, tmp_path):
    res = run_binary("optuna_physics_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "Best Trial" in res.stdout


def test_f3_optuna_physics_tuner_invalid_arg(run_binary):
    res = run_binary("optuna_physics_tuner", ["--bogus_flag", "1"])
    assert res.returncode != 0


def test_f3_optuna_physics_tuner_zero_trials(run_binary, tmp_path):
    res = run_binary("optuna_physics_tuner", ["--n_trials", "0"], cwd=str(tmp_path))
    assert res.returncode != 0


# Feature 4: Optuna gameplay tuner
def test_f4_optuna_gameplay_tuner_help(run_binary):
    res = run_binary("optuna_gameplay_tuner", ["--help"])
    assert res.returncode == 0
    assert "--n_trials" in res.stdout


def test_f4_optuna_gameplay_tuner_basic(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode == 0
    assert "parameters.proto" in res.stdout
    assert os.path.isfile(tmp_path / "gameplay_tuner.db")


def test_f4_optuna_gameplay_tuner_invalid_arg(run_binary):
    res = run_binary("optuna_gameplay_tuner", ["--bogus_flag", "1"])
    assert res.returncode != 0


def test_f4_optuna_gameplay_tuner_zero_trials(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "0"], cwd=str(tmp_path))
    assert res.returncode != 0


def test_f4_optuna_gameplay_tuner_negative_trials(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "-1"], cwd=str(tmp_path))
    assert res.returncode != 0


# Feature 5: Unified CLI (auto_tune)
def test_f5_auto_tune_help(run_binary):
    res = run_binary("auto_tune", ["--help"])
    assert res.returncode == 0
    assert "--pipeline" in res.stdout
    assert "--logs_dir" in res.stdout
    assert "--n_trials" in res.stdout


def test_f5_auto_tune_invalid_pipeline(run_binary):
    res = run_binary("auto_tune", ["--pipeline", "invalid_mode"])
    assert res.returncode != 0


def test_f5_auto_tune_dispatches_sim2real_only(run_binary, tmp_path):
    res = run_binary(
        "auto_tune", ["--pipeline", "sim2real", "--n_trials", "1"], cwd=str(tmp_path)
    )
    assert "Starting sim2real physics tuning pipeline" in res.stdout
    assert "Starting gameplay tuning pipeline" not in res.stdout


def test_f5_auto_tune_dispatches_gameplay_only(run_binary, tmp_path):
    res = run_binary(
        "auto_tune", ["--pipeline", "gameplay", "--n_trials", "1"], cwd=str(tmp_path)
    )
    assert "Starting gameplay tuning pipeline" in res.stdout
    assert "Starting sim2real physics tuning pipeline" not in res.stdout


def test_f5_auto_tune_dispatches_both_for_all(run_binary, tmp_path):
    res = run_binary("auto_tune", ["--n_trials", "1"], cwd=str(tmp_path))
    assert "Starting sim2real physics tuning pipeline" in res.stdout
    assert "Starting gameplay tuning pipeline" in res.stdout


# Feature 6: Pausable execution (Optuna SQLite resume)
def test_f6_physics_tuner_creates_db(dummy_log_dir, run_binary, tmp_path):
    res = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert res.returncode == 0
    assert os.path.isfile(tmp_path / "physics_tuner.db")


def test_f6_physics_tuner_resumes_from_db(dummy_log_dir, run_binary, tmp_path):
    res1 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    res2 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", dummy_log_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert res1.returncode == 0
    assert res2.returncode == 0
    assert "Using an existing study" in res2.stderr


def test_f6_gameplay_tuner_creates_db(run_binary, tmp_path):
    res = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode == 0
    assert os.path.isfile(tmp_path / "gameplay_tuner.db")


def test_f6_gameplay_tuner_resumes_from_db(run_binary, tmp_path):
    res1 = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    res2 = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res1.returncode == 0
    assert res2.returncode == 0
    assert "Using an existing study" in res2.stderr


def test_f6_physics_tuner_corrupt_db(run_binary, tmp_path):
    (tmp_path / "physics_tuner.db").write_text("not a sqlite database")
    res = run_binary("optuna_physics_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res.returncode != 0


# Feature 7: Bazel binary / runfiles integration
def test_f7_all_binaries_present_and_executable(run_binary):
    for binary_name in [
        "extract_kinematics_events",
        "extract_game_events",
        "optuna_physics_tuner",
        "optuna_gameplay_tuner",
        "auto_tune",
    ]:
        binary_path = os.path.abspath(os.path.join("software/sim_to_real", binary_name))
        assert os.path.isfile(binary_path)
        assert os.access(binary_path, os.X_OK)


def test_f7_auto_tune_runs_from_arbitrary_cwd(run_binary, tmp_path):
    res = run_binary("auto_tune", ["--help"], cwd=str(tmp_path))
    assert res.returncode == 0


def test_f7_extractor_respects_build_workspace_directory(
    dummy_log_dir, run_binary, tmp_path
):
    workspace_dir = tmp_path / "workspace"
    workspace_dir.mkdir()
    res = run_binary(
        "extract_kinematics_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(workspace_dir)},
    )
    assert res.returncode == 0
    assert os.path.isfile(workspace_dir / "software/sim_to_real/kinematics_world.csv")


if __name__ == "__main__":
    import sys
    import pytest

    sys.exit(pytest.main(sys.argv))
