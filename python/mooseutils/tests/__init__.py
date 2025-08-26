import os
from mooseutils import find_moose_directory

MOOSE_DIR = find_moose_directory() or os.getcwd()
TEST_FILES = os.path.join(MOOSE_DIR, "python", "test_files")
