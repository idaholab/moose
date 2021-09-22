#!/usr/bin/env python3

from moosetools import chigger

window = chigger.Window(observer=True)
viewport = chigger.Viewport()
reader = chigger.exodus.ExodusReader(filename='../tests/input/mug_blocks_out.e')
source = chigger.exodus.ExodusSource(variable='convected')

window.render()

import vtk
recorder = vtk.vtkInteractorEventRecorder()
recorder.SetInteractor(window.getVTKInteractor())
recorder.SetFileName('record.log')
recorder.Record()

window.start()
