import gzip
import os
import shutil
import subprocess
import tempfile

import pytest


@pytest.fixture
def dummy_log_dir():
    """A temp directory containing a single valid (but empty) .replay file.

    An empty gzip file is a structurally valid replay chunk (version
    defaults to 1 and it contains zero log entries), so the extractors
    process it successfully and produce empty datasets.
    """
    path = tempfile.mkdtemp()
    with gzip.open(os.path.join(path, "0.replay"), "wb"):
        pass
    yield path
    shutil.rmtree(path, ignore_errors=True)


@pytest.fixture
def empty_log_dir():
    """A temp directory containing no .replay files."""
    path = tempfile.mkdtemp()
    yield path
    shutil.rmtree(path, ignore_errors=True)


@pytest.fixture
def run_binary():
    """Run a //software/sim_to_real py_binary built by Bazel.

    The `data` deps on these binaries place their launcher scripts at
    software/sim_to_real/<name> relative to the test's runfiles root, which
    is also the working directory pytest is started in under `bazel test`.
    """

    def _run_binary(binary_name, args, cwd=None, env=None):
        binary_path = os.path.abspath(
            os.path.join("software/sim_to_real", binary_name)
        )
        run_env = None
        if env is not None:
            run_env = dict(os.environ)
            run_env.update(env)
        return subprocess.run(
            [binary_path] + args,
            capture_output=True,
            text=True,
            cwd=cwd,
            env=run_env,
        )

    return _run_binary
