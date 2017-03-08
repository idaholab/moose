#!/usr/bin/env python
#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
import vtk
import chigger

camera = vtk.vtkCamera()
camera.SetViewUp(-0.865080, 0.018659, 0.501287)
camera.SetPosition(6.368139, -8.683361, 11.437825)
camera.SetFocalPoint(0.000000, 0.000000, 0.125000)

reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e')

tclip = chigger.filters.PlaneClipper(normal=[0,0,1])
top = chigger.exodus.ExodusResult(reader, camera=camera, variable='diffused', cmap='viridis', opacity=0.5, range=[0,2], filters=[tclip])

bclip = chigger.filters.PlaneClipper(normal=[0,0,-1])
bottom = chigger.exodus.ExodusResult(reader, camera=camera, variable='diffused', cmap='viridis', range=[0,2], filters=[bclip])

window = chigger.RenderWindow(bottom, top, size=[600,600], test=True)
window.write('multiple_clips.png')
window.start()
