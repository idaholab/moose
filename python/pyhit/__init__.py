import os
import sys
import subprocess
import mooseutils

try:
    import hit

except ImportError:

    # If the repository is not a git repository, then give up otherwise try to figure out what to do
    if not mooseutils.is_git_repo(os.path.dirname(__file__)):
        raise

    moose_dir = mooseutils.git_root_dir(os.path.dirname(__file__))
    status = mooseutils.git_submodule_info(moose_dir)

    # Use framework/contrib/hit because moosetools submodule is not available
    if status['moosetools'][0] == '-':
        hit_dir = os.path.join(moose_dir, 'framework', 'contrib', 'hit')
        sys.path.append(hit_dir)
        try:
            import hit
        except ImportError:
            moose_test_dir = os.path.abspath(os.path.join(moose_dir, 'test'))
            subprocess.run(['make', 'hit'], cwd=moose_test_dir)
            import hit
    # Use hit in moosetools submodule
    else:
        hit_dir = os.path.join(moose_dir, 'moosetools', 'contrib', 'hit')
        sys.path.append(hit_dir)
        try:
            import hit
        except ImportError:
            subprocess.run(['make', 'hit.so'], cwd=hit_dir)
            import hit

from hit import TokenType, Token
from .pyhit import Node, load, write, parse, tokenize
