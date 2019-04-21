#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function
import os
import glob
import shutil
import subprocess
import numpy as np
import vtk
import mooseutils
from .Options import Option, Options
from . import AxisOptions
from . import FontOptions
from . import LegendOptions

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
    basename = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'tests', 'input',
                                            'step10_micro_out.e'))
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

def print_camera(camera, prefix='camera', precision=10):
    """
    Prints vtkCamera object to screen.
    """
    if not isinstance(camera, vtk.vtkCamera):
        print("You must supply a vtkCarmera object.")
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

    return [prefix + '.SetViewUp' + dump(precision, view_up), prefix + '.SetPosition' + \
                                         dump(precision, position), prefix + '.SetFocalPoint' + \
                                         dump(precision, focal)]

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

def img2mov(pattern, output, ffmpeg='ffmpeg', duration=60, framerate=None, bitrate='10M',
            num_threads=1, quality=1, dry_run=False, output_framerate_increase=0):
    """
    Use ffmpeg to convert a series of images to a movie.

    Args:
        pattern[str]: The glob pattern defining the files to be converted.
        output[str]: The name of the output file, including the extension.
        ffmpeg[str]: The ffmpeg executable.
        duration[int]: The desired duration of the movie (in seconds)
        framerate[int]: Ignores the duration and sets the movie framerate directly.
        bitrate[str]: The ffmeg "-b:v" setting.
        num_threads[int]: The number of threads to utilize in rendering.
        quality[int]: The ffmpeg quality setting (ranges from 1 to 31).
        dry_run[bool]: When True the command is not executed.
        factor[float]: Output framerate adjustment to help guarantee no dropped frames, if you see
                       dropped frames in ffmpeg output, increase this number.
    """

    # Compute framerate from the duration if framerate is not given
    if not framerate:
        n = len(glob.glob(pattern))
        framerate = n/duration

    # Build the command
    cmd = [ffmpeg]
    cmd += ['-pattern_type', 'glob']
    cmd += ['-framerate', str(framerate)]
    cmd += ['-i', pattern]
    cmd += ['-b:v', bitrate]
    cmd += ['-pix_fmt', 'yuv420p']
    cmd += ['-q:v', str(quality)]
    cmd += ['-threads', str(num_threads)]
    cmd += ['-framerate', str(framerate + output_framerate_increase)]
    cmd += [output]

    c = ' '.join(cmd)
    print('{0}\n{1}\n{0}'.format('-'*(len(c)), c))
    if not dry_run:
        subprocess.call(cmd)
