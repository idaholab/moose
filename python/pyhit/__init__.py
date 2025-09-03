import os
import sys
import subprocess
from mooseutils import find_moose_directory

# First, attempt to import hit. If that fails, try adding the hit source directory to the path
# and try the import again. If that fails, try running "make hit" before importing.
try:
    import hit  # noqa: F401
except ModuleNotFoundError as e:
    moose_dir = find_moose_directory()
    if moose_dir is None:
        raise e
    hit_dir = os.path.join(moose_dir, "framework", "contrib", "hit")
    if not os.path.isdir(hit_dir):
        raise e
    sys.path.append(hit_dir)
    try:
        import hit  # noqa: F401
    except ImportError:
        moose_test_dir = os.path.abspath(os.path.join(moose_dir, "test"))
        subprocess.run(["make", "hit"], cwd=moose_test_dir)
        import hit  # noqa: F401

from hit import TokenType as TokenType, Token as Token
from .pyhit import (
    Node as Node,
    load as load,
    write as write,
    parse as parse,
    tokenize as tokenize,
)
