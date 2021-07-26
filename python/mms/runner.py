#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import copy
import mooseutils
import pandas

SPATIAL = 0
TEMPORAL = 1

def _runner(input_files, num_refinements, *args, **kwargs):
    """
    Helper class for running MOOSE-based application input file for convergence study.

    Inputs:
        input_file(s)[str|list]: The name of the input file(s) run
        num_refinements: The number of refinements to perform

    Optional Key-Value Options:
        x_pp[str]: The Postprocessor to use for the x-axis; ignored for rtype=TEMPORAL (default: 'h')
        y_pp[str]: The Postprocessor to use for the y-axis (default: 'error')
        executable[str]: The executable to run, if not provided the executable is automatically
                         detected
        csv[str]: The name of the CSV containing the Postprocessor data, if not provided the name
                  is assumed to be the standard output name if Outputs/csv=true in the input file
        rtype[int]: SPATIAL or TEMPORAL
        dt[float]: The initial timestep, only used with rtype=TEMPORAL (default: 1)
        file_base[str]: A string pattern for outputting files

    All additional arguments are passed to the executable
    """

    x_pp = kwargs.get('x_pp', 'h')
    y_pp = kwargs.get('y_pp', ['error'])

    if not isinstance(y_pp, list):
        y_pp = [y_pp]

    executable = kwargs.get('executable', None)
    csv = kwargs.get('csv', None)
    console = kwargs.get('console', True)
    mpi = kwargs.get('mpi', None)
    rtype = kwargs.get('rtype') # SPATIAL or TEMPORAL
    dt = kwargs.pop('dt', 1) # only used with rtype=TEMPORAL
    file_base = kwargs.pop('file_base', None)

    # Create list of input_files, if single file provided
    if isinstance(input_files, str): input_files = [input_files]

    # Check that input file exists
    for input_file in input_files:
        if not os.path.isfile(input_file):
            raise IOError("The supplied file, '{}', does not exist.".format(input_file))

    # Assume output CSV file, if not specified
    if csv is None:
        fcsv = input_files[-1].replace('.i', '_out.csv')

    # Locate the executable
    if executable is None:
        executable = mooseutils.find_moose_executable_recursive(os.getcwd())
    elif os.path.isdir(executable):
        executable = mooseutils.find_moose_executable(executable)
    elif not os.path.isfile(executable):
        raise IOError("Unable to locate executable: {}".format(executable))

    if executable is None:
        raise IOError("No application executable found.")

    # Build custom arguments
    cli_args = ['-i'] + input_files
    cli_args += args

    # Run input file and build up output
    x = []
    y = [ [] for _ in range(len(y_pp)) ]

    if not isinstance(num_refinements, list):
        num_refinements = list(range(num_refinements))

    for step in num_refinements:
        a = copy.copy(cli_args)
        if rtype == SPATIAL:
            a.append('Mesh/uniform_refine={}'.format(step))
        elif rtype == TEMPORAL:
            a.append('Executioner/dt={}'.format(dt))
            dt = dt / 2.

        if file_base:
            fbase = file_base.format(step)
            a.append('Outputs/file_base={}'.format(fbase))
            if csv is None:
                fcsv = '{}.csv'.format(fbase)

        print('Running:', executable, ' '.join(a))
        out = mooseutils.run_executable(executable, *a, mpi=mpi, suppress_output=not console)

        # Check that CSV file exists
        if not os.path.isfile(fcsv):
            raise IOError("The CSV output does not exist: {}".format(csv))

        # Load data for h and error
        current = pandas.read_csv(fcsv)

        if rtype == SPATIAL:
            x.append(current[x_pp].iloc[-1])
            for index,pp in enumerate(y_pp):
                y[index].append(current[pp].iloc[-1])
        elif rtype == TEMPORAL:
            x.append(dt)
            for index,pp in enumerate(y_pp):
                y[index].append(current[pp].iloc[-1])

    if rtype == SPATIAL:
        x_pp == 'dt'

    df_dict = {x_pp:x}
    df_columns = [x_pp]
    for i in range(len(y_pp)):
        df_dict.update({y_pp[i]:y[i]})
        df_columns.append(y_pp[i])

    return pandas.DataFrame(df_dict, columns=df_columns)

def run_spatial(*args, **kwargs):
    """Runs input file for a spatial MMS problem (see _runner.py for inputs)."""
    return _runner(*args, rtype=SPATIAL, **kwargs)

def run_temporal(*args, **kwargs):
    """Runs input file for a temporal MMS problem (see _runner.py for inputs)."""
    return _runner(*args, rtype=TEMPORAL, **kwargs)
