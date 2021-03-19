import os
import sys
import subprocess
import mooseutils

moose_dir = mooseutils.git_root_dir(os.path.dirname(__file__))
status = mooseutils.git_submodule_status(moose_dir)

# Use framework/contrib/hit because moosetools submodule is not available
if status['moosetools'] == '-':
    try:
        from . import hit
    except:
        moose_test_dir = os.path.abspath(os.path.join(moose_dir, 'test'))
        subprocess.run(['make', 'hit'], cwd=moose_test_dir)
        from . import hit
# Use hit in moosetools submodule
else:
    hit_dir = os.path.join(moose_dir, 'moosetools', 'contrib', 'hit')
    try:
        sys.path.append(hit_dir)
        import hit
    except:
        subprocess.run(['make', 'hit.so'], cwd=hit_dir)
        import hit

from hit import TokenType, Token
from .pyhit import Node, load, write, parse, tokenize
