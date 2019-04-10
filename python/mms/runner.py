import os
import copy
import mooseutils
import pandas

SPATIAL = 0
TEMPORAL = 1

def _runner(input_file, num_refinements, *args, **kwargs):
    """
    Helper class for running MOOSE-based application input file for convergence study.

    Inputs:
        input_file[str]: The name of the input file to run
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

    All additional arguments are passed to the executable
    """

    x_pp = kwargs.get('x_pp', 'h')
    y_pp = kwargs.get('y_pp', 'error')
    executable = kwargs.get('executable', None)
    csv = kwargs.get('csv', None)
    console = kwargs.get('console', True)
    rtype = kwargs.get('rtype') # SPATIAL or TEMPORAL
    dt = kwargs.pop('dt', 1) # only used with rtype=TEMPORAL

    # Check that input file exists
    if not os.path.isfile(input_file):
        raise IOError("The supplied file, '{}', does not exist.".format(input_file))

    # Assume output CSV file, if not specified
    if csv is None:
        csv = input_file.replace('.i', '_out.csv')

    # Locate the executable
    if executable is None:
        executable = mooseutils.find_moose_executable_recursive(os.getcwd())

    if executable is None:
        raise IOError("No application executable found.")

    # Build custom arguments
    cli_args = ['-i', input_file]
    cli_args += args

    # Run input file and build up output
    x = []
    y = []
    for step in xrange(0, num_refinements):
        a = copy.copy(cli_args)
        if rtype == SPATIAL:
            a.append('Mesh/uniform_refine={}'.format(step))
        elif rtype == TEMPORAL:
            a.append('Executioner/dt={}'.format(dt))
            dt = dt / 2.

        print 'Running:', executable, ' '.join(a)
        out = mooseutils.run_executable(executable, a, suppress_output=not console)

        # Check that CSV file exists
        if not os.path.isfile(csv):
            raise IOError("The CSV output does not exist: {}".format(csv))

        # Load data for h and error
        current = pandas.read_csv(csv)

        if rtype == SPATIAL:
            x.append(current[x_pp].iloc[-1])
            y.append(current[y_pp].iloc[-1])
        elif rtype == TEMPORAL:
            x.append(dt)
            y.append(current[y_pp].iloc[-1])

    if rtype == SPATIAL:
        x_pp == 'dt'

    return pandas.DataFrame({x_pp:x, y_pp:y}, columns=[x_pp, y_pp])

def run_spatial(*args, **kwargs):
    """Runs input file for a spatial MMS problem (see _runner.py for inputs)."""
    return _runner(*args, rtype=SPATIAL, **kwargs)

def run_temporal(*args, **kwargs):
    """Runs input file for a temporal MMS problem (see _runner.py for inputs)."""
    return _runner(*args, rtype=TEMPORAL, **kwargs)
