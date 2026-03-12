import sys
import os
import importlib

from pybind11_stubgen import main

if __name__ == '__main__':
    stub_path = os.environ.get("STUB_PYTHONPATH")
    if stub_path:
        # We need the directory containing `software`, which is the `bazel-out/.../bin` root.
        sys.path.insert(0, stub_path)

    # Let's try to import the raw .so file as a module using importlib
    try:
        if len(sys.argv) > 1:
            module_name = sys.argv[-1]
            if not module_name.startswith("-"):
                # First attempt standard import
                try:
                    importlib.import_module(module_name)
                except ImportError:
                    print(f"Standard import failed for {module_name}, attempting direct load from {stub_path}...")
                    
                    if stub_path:
                        import importlib.util
                        module_parts = module_name.split('.')
                        # Construct expected .so path (e.g. software/py_constants.so)
                        so_path = os.path.join(stub_path, *module_parts) + ".so"
                        print(f"Looking for SO file at: {so_path}")
                        
                        if os.path.exists(so_path):
                            spec = importlib.util.spec_from_file_location(module_name, so_path)
                            module = importlib.util.module_from_spec(spec)
                            sys.modules[module_name] = module
                            spec.loader.exec_module(module)
                            print(f"Successfully loaded {module_name} directly from {so_path}")
                        else:
                            raise ImportError(f"Could not find {so_path}")
                    else:
                        raise
    except Exception as e:
        print(f"Error importing {module_name}: {e}")
        print(f"sys.path: {sys.path}")
        sys.exit(1)
        
    sys.exit(main())