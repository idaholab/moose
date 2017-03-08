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

# Create a common camera
camera = vtk.vtkCamera()
camera.SetViewUp(0.3405, 0.8395, -0.4234)
camera.SetPosition(-1.7356, 2.8002, 3.3410)
camera.SetFocalPoint(0.8096, 0.3052, 0.4410)

reader = chigger.exodus.MultiAppExodusReader('../input/multiapps_out_sub*.e')
multiapp = chigger.exodus.ExodusResult(reader, variable='u', cmap='coolwarm', camera=camera)

window = chigger.RenderWindow(multiapp, size=[300,300], test=True)
window.update()
window.resetCamera() # TODO: This is needed to re-center the object, not sure why
window.write('multiapp.png')
window.start()
