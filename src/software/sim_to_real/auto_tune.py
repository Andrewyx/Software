import sys
import subprocess
import argparse


def run_sim2real(logs_dir, n_trials):
    """Run the physics (sim-to-real) tuning pipeline via bazel."""
    print("=" * 60)
    print("[auto_tune] Starting sim2real physics tuning pipeline...")
    print("=" * 60)
    result = subprocess.run(
        [
            "bazel",
            "run",
            "//software/sim_to_real:optuna_physics_tuner",
            "--",
            "--logs_dir",
            logs_dir,
            "--n_trials",
            str(n_trials),
        ],
    )
    if result.returncode != 0:
        print(f"[auto_tune] sim2real pipeline exited with code {result.returncode}")
    else:
        print("[auto_tune] sim2real pipeline completed successfully.")
    return result.returncode


def run_gameplay(n_trials):
    """Run the gameplay tuning pipeline via bazel."""
    print("=" * 60)
    print("[auto_tune] Starting gameplay tuning pipeline...")
    print("=" * 60)
    result = subprocess.run(
        [
            "bazel",
            "run",
            "//software/sim_to_real:optuna_gameplay_tuner",
            "--",
            "--n_trials",
            str(n_trials),
        ],
    )
    if result.returncode != 0:
        print(f"[auto_tune] gameplay pipeline exited with code {result.returncode}")
    else:
        print("[auto_tune] gameplay pipeline completed successfully.")
    return result.returncode


def main():
    parser = argparse.ArgumentParser(
        description="Automated Sim-to-Real Tuning Pipeline"
    )
    parser.add_argument(
        "--pipeline",
        type=str,
        choices=["sim2real", "gameplay", "all"],
        default="all",
        help="Which tuning pipeline to run (default: all)",
    )
    parser.add_argument(
        "--logs_dir",
        type=str,
        default=".",
        help="Directory containing kinematics CSV logs (default: '.')",
    )
    parser.add_argument(
        "--n_trials",
        type=int,
        default=10,
        help="Number of Optuna trials per pipeline (default: 10)",
    )
    args = parser.parse_args()

    exit_codes = []

    if args.pipeline in ("sim2real", "all"):
        exit_codes.append(run_sim2real(args.logs_dir, args.n_trials))

    if args.pipeline in ("gameplay", "all"):
        exit_codes.append(run_gameplay(args.n_trials))

    if any(code != 0 for code in exit_codes):
        print("\n[auto_tune] WARNING: One or more pipelines failed.")
        sys.exit(1)
    else:
        print("\n[auto_tune] All pipelines completed successfully.")


if __name__ == "__main__":
    main()
