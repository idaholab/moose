#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, shlex, subprocess, sys, urllib.parse

# This is a helper script for running an external process in HPC _not_
# within a shell, which allows for continuity of running things on HPC
# just like we run them within the SubprocessRunner. It allows us to not
# deal with escaping all kinds of crud as we execute it within a shell.
# It takes a single argument, which is the url encoded thing to run,
# decodes it, and runs it.
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('ERROR: Expected single argument of the encoded command to run')
        sys.exit(1)

    # The command should be encoded on other end with urrllib.parse.quote
    encoded_command = sys.argv[1]
    command = shlex.split(urllib.parse.unquote(encoded_command))

    # Try to only print this on rank 0
    rank = os.environ.get('PMI_RANK') # mpich
    if rank is None:
        rank = os.environ.get('OMPI_COMM_WORLD_RANK') # openmpi
    if rank == '0' or rank is None:
        print('Running decoded command:', ' '.join(command), flush=True)

    # Run the thing; close_fds=False needed for MPI
    process = subprocess.run(command,
                             stdout=sys.stdout,
                             stderr=sys.stderr,
                             close_fds=False)
    # This is a wrapper so just exit with the code of whatever we ran
    sys.exit(process.returncode)
