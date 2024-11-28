import os
import subprocess

Import("env")

def run_clang_format(source_dir):
    for root, _, files in os.walk(source_dir):
        for file in files:
            if file.endswith(('.c', '.h')):
                filepath = os.path.join(root, file)
                print(f"formatting {filepath}")
                subprocess.run(["clang-format", "-i", filepath], check=True)

def before_build(source, target, env):
    project_src_dir = env['PROJECT_SRC_DIR']
    project_include_dir = env['PROJECT_INCLUDE_DIR']
    run_clang_format(project_src_dir)
    run_clang_format(project_include_dir)

env.AddPreAction("buildprog", before_build)
