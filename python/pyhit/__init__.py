import os
import subprocess

try:
    from . import hit
except:
    testdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'test'))
    subprocess.run(['make', 'hit'], cwd=testdir)
    from . import hit

from hit import TokenType, Token
from .pyhit import Node, load, write, parse, tokenize
