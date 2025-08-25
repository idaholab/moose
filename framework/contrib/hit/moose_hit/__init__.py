import sys
from importlib import resources
import subprocess

def main():
    hit_exe = resources.files("moose_hit").joinpath("hit")
    argv = [str(hit_exe), *sys.argv[1:]]
    sys.exit(subprocess.run(argv).returncode)
