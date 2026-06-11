import os


# 1. extract_kinematics_events -> optuna_physics_tuner (Feature 1 + Feature 3)
def test_combo_extract_kinematics_then_physics_tune(dummy_log_dir, run_binary, tmp_path):
    extract_res = run_binary(
        "extract_kinematics_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert extract_res.returncode == 0

    csv_dir = tmp_path / "software/sim_to_real"
    assert os.path.isfile(csv_dir / "kinematics_world.csv")

    tune_res = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", str(csv_dir), "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert tune_res.returncode == 0
    assert "Best Trial" in tune_res.stdout
    assert os.path.isfile(tmp_path / "physics_tuner.db")


# 2. extract_game_events -> optuna_gameplay_tuner (Feature 2 + Feature 4)
def test_combo_extract_game_then_gameplay_tune(dummy_log_dir, run_binary, tmp_path):
    extract_res = run_binary(
        "extract_game_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert extract_res.returncode == 0

    csv_dir = tmp_path / "software/sim_to_real"
    assert os.path.isfile(csv_dir / "game_passes.csv")
    assert os.path.isfile(csv_dir / "game_interceptions.csv")

    tune_res = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert tune_res.returncode == 0
    assert "Best Trial" in tune_res.stdout
    assert os.path.isfile(tmp_path / "gameplay_tuner.db")


# 3. optuna_physics_tuner resume across runs (Feature 3 + Feature 6)
def test_combo_physics_tuner_resume_with_extracted_data(dummy_log_dir, run_binary, tmp_path):
    extract_res = run_binary(
        "extract_kinematics_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert extract_res.returncode == 0

    csv_dir = str(tmp_path / "software/sim_to_real")

    res1 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", csv_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    res2 = run_binary(
        "optuna_physics_tuner",
        ["--logs_dir", csv_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert res1.returncode == 0
    assert res2.returncode == 0
    assert "Using an existing study" in res2.stderr


# 4. optuna_gameplay_tuner resume across runs (Feature 4 + Feature 6)
def test_combo_gameplay_tuner_resume_with_extracted_data(dummy_log_dir, run_binary, tmp_path):
    extract_res = run_binary(
        "extract_game_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert extract_res.returncode == 0

    res1 = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    res2 = run_binary("optuna_gameplay_tuner", ["--n_trials", "1"], cwd=str(tmp_path))
    assert res1.returncode == 0
    assert res2.returncode == 0
    assert "Using an existing study" in res2.stderr


# 5. extract_kinematics_events + extract_game_events share a workspace (Feature 1 + Feature 2)
def test_combo_both_extractors_share_workspace(dummy_log_dir, run_binary, tmp_path):
    kinematics_res = run_binary(
        "extract_kinematics_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    game_res = run_binary(
        "extract_game_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert kinematics_res.returncode == 0
    assert game_res.returncode == 0

    csv_dir = tmp_path / "software/sim_to_real"
    assert os.path.isfile(csv_dir / "kinematics_world.csv")
    assert os.path.isfile(csv_dir / "kinematics_intent.csv")
    assert os.path.isfile(csv_dir / "game_passes.csv")
    assert os.path.isfile(csv_dir / "game_interceptions.csv")


# 6. auto_tune dispatches both pipelines in order (Feature 5 + Feature 3 + Feature 4)
def test_combo_auto_tune_dispatch_order(run_binary, tmp_path):
    res = run_binary("auto_tune", ["--n_trials", "1"], cwd=str(tmp_path))
    sim2real_idx = res.stdout.find("Starting sim2real physics tuning pipeline")
    gameplay_idx = res.stdout.find("Starting gameplay tuning pipeline")
    assert sim2real_idx != -1
    assert gameplay_idx != -1
    assert sim2real_idx < gameplay_idx


# 7. extract_kinematics_events -> auto_tune --pipeline sim2real (Feature 1 + Feature 5)
def test_combo_extract_then_auto_tune_sim2real(dummy_log_dir, run_binary, tmp_path):
    extract_res = run_binary(
        "extract_kinematics_events",
        [dummy_log_dir],
        env={"BUILD_WORKSPACE_DIRECTORY": str(tmp_path)},
    )
    assert extract_res.returncode == 0

    csv_dir = str(tmp_path / "software/sim_to_real")

    res = run_binary(
        "auto_tune",
        ["--pipeline", "sim2real", "--logs_dir", csv_dir, "--n_trials", "1"],
        cwd=str(tmp_path),
    )
    assert "Starting sim2real physics tuning pipeline" in res.stdout
    assert "Starting gameplay tuning pipeline" not in res.stdout


if __name__ == "__main__":
    import sys
    import pytest

    sys.exit(pytest.main(sys.argv))
