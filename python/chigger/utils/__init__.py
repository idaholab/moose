"""utility function and classes for chigger"""
import os
import glob
import shutil
import subprocess
import numpy as np
import vtk
from Options import Option, Options
import AxisOptions
import FontOptions
import LegendOptions

def get_active_filenames(basename, pattern=None):
    """
    Return a list of tuples containing 'active' filenames and modified times.

    Inputs:
        basename[str]: The base filename (e.g., file_out.e)
        pattern[str]: (Optional) Additional files to consider via glob pattern (e.g., file_out.e-s*)
    """

    # List of all matching filenames
    filenames = [basename]
    if pattern:
        filenames += glob.glob(pattern)
    filenames.sort()

    # Minimum filename modified time
    modified = os.path.getmtime(filenames[0]) if os.path.exists(filenames[0]) else 0

    # Populate a list of tuples: (filename, modified time)
    output = []
    for filename in filenames:
        current_modified = os.path.getmtime(filename) if os.path.exists(filename) else 0
        if current_modified >= modified:
            output.append((filename, current_modified))

    return output

def copy_adaptive_exodus_test_files(testbase):
    """
    A helper for copying test Exodus files.
    """
    basename = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'tests', 'input', 'step10_micro_out.e'))
    pattern = basename + '-s*'

    testfiles = []
    for src in [basename] + glob.glob(pattern):
        _, ext = os.path.splitext(src)
        dst = os.path.join(os.getcwd(), testbase + ext)
        testfiles.append(dst)
        shutil.copy(src, dst)
    return sorted(testfiles)

def get_bounds_min_max(*all_bounds):
    """
    Returns min,max bounds arrays provided a list of bounds sets.
    """
    xmin = [float('inf'), float('inf'), float('inf')]
    xmax = [float('-inf'), float('-inf'), float('-inf')]

    for bounds in all_bounds:
        for i, j in enumerate([0, 2, 4]):
            xmin[i] = min(xmin[i], bounds[j])
        for i, j in enumerate([1, 3, 5]):
            xmax[i] = max(xmax[i], bounds[j])
    return xmin, xmax

def get_bounds(*sources):
    """
    Returns the bounding box for all supplied sources.
    """
    bnds = []
    for src in sources:
        bnds.append(src.getVTKMapper().GetBounds())
    return get_bounds_min_max(*bnds)

def compute_distance(*sources):
    """
    Returns the distance across the bounding box for all supplied sources.
    """
    xmin, xmax = get_bounds(*sources)
    return np.linalg.norm(np.array(xmax) - np.array(xmin))

def get_min_max(*pairs):
    """
    Retuns the min/max from a set of min/max pairs.
    """
    xmin = float('inf')
    xmax = float('-inf')
    for x0, x1 in pairs:
        xmin = min(xmin, x0)
        xmax = max(xmax, x1)
    return xmin, xmax

def print_camera(camera, prefix='camera', precision=4):
    """
    Prints vtkCamera object to screen.
    """
    if not isinstance(camera, vtk.vtkCamera):
        print "You must supply a vtkCarmera object."
        return

    view_up = camera.GetViewUp()
    position = camera.GetPosition()
    focal = camera.GetFocalPoint()

    def dump(precision, vec):
        """
        Helper for dumping settings.
        """
        p = str(precision)
        frmt = ''.join(['{:', p, '.', p, 'f}'])

        d = ''.join(['(', frmt, ', ', frmt, ', ', frmt, ')'])
        return d.format(*vec)

    return [prefix + '.SetViewUp' + dump(precision, view_up), prefix + '.SetPosition' + dump(precision, position), prefix + '.SetFocalPoint' + dump(precision, focal)]

def animate(pattern, output, delay=20, restart_delay=500, loop=True):
    """
    Runs ImageMagic convert to create an animate gif from a series of images.
    """
    filenames = glob.glob(pattern)
    delay = [delay]*len(filenames)
    delay[-1] = restart_delay
    cmd = ['convert']
    for d, f in zip(delay, filenames):
        cmd += ['-delay', str(d), f]
    if loop:
        cmd += ['-loop', '0']
    cmd += [output]
    subprocess.call(cmd)
