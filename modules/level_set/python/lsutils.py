#!/usr/bin/env/python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import glob
import vtk
import argparse
import chigger


def build_frames(camera=None):
    """
    Function for building level set demonstration images/movies.

    Args:
      camera[vtkCamera]: The camera to utilize for the rendered window.
    """

    parser = argparse.ArgumentParser(description="Process results for initial/final condition and/or area conservation.")
    parser.add_argument('exodus', help="The ExodusII file to process.")
    parser.add_argument('--variable', '-v', default='phi', type=str, help="The variable to display.")
    parser.add_argument('--interval', default=5, type=int, help="The image output interval.")
    parser.add_argument('--output', '-o', type=str, help="The prefix for the outputted PNG files.")
    parser.add_argument('--size', default=[400, 400], type=list, help="The image size, in pixels.")
    parser.add_argument('--gif', action='store_true', help="Enable the creation of an animated gif via ImageMagick, using the --output prefix for the filename.")
    parser.add_argument('--delay', default=20, type=int, help="Delay between frames in animate gif.")
    parser.add_argument('--clean', action='store_true', help="Delete existing PNG files matching the output name.")
    options = parser.parse_args()

    # Initial Condition
    initial_reader = chigger.exodus.ExodusReader(options.exodus, timestep=0)
    initial_contour = chigger.filters.ContourFilter(levels=[0.5])
    initial_result = chigger.exodus.ExodusResult(initial_reader, variable=options.variable, camera=camera, color=[0,0,0], filters=[initial_contour], edge_width=2, layer=2)

    # The background "phi" colormap
    reader = chigger.exodus.ExodusReader(options.exodus)
    reader.update()
    result = chigger.exodus.ExodusResult(reader, variable=options.variable, camera=camera, layer=1, opacity=0.8, cmap='viridis', edges=True, edge_color=[0.5,0.5,0.5])

    # The current contour
    result_contour = chigger.filters.ContourFilter(levels=[0.5])
    result2 = chigger.exodus.ExodusResult(reader, variable=options.variable, camera=camera, color=[0,1,0], layer=2, edge_width=2, filters=[result_contour])

    # Write image frames
    window = chigger.RenderWindow(initial_result, result, result2, background=[1,1,1], size=[400,400], style='interactive2D')

    # Determine output
    if not options.output:
        options.output, _ = os.path.splitext(options.exodus)

    # Clean existing PNGs
    if options.clean:
        for filename in glob.glob('{}*.png'.format(options.output)):
            os.remove(filename)

    # Write the frames
    n = len(reader.getTimes())
    rng = range(0, n, options.interval) + [n-1]
    for i in rng:
        reader.setOptions(timestep=i)
        window.write('{}_{:04d}.png'.format(options.output, i))
        window.update()

    # Create animated _gif
    if options.gif:
        chigger.utils.animate('{}_*.png'.format(options.output), '{}.gif'.format(options.output), delay=options.delay)
