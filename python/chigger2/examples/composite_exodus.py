#!/usr/bin/env python3

from moosetools import chigger

window = chigger.Window()
viewport = chigger.Viewport()

#reader0 = chigger.exodus.ExodusReader(filename='../tests/input/multiapps_out_sub0.e')
reader = chigger.exodus.ExodusCompositeReader(pattern='../tests/input/multiapps_out_sub*.e')
source = chigger.exodus.ExodusSource(variable='u')

#reader = chigger.exodus.ExodusCompositeReader(pattern='../tests/input/multiapps_out_sub*.e')
#source = chigger.exodus.ExodusCompositeSource(reader=reader, variable='u')

#window.render()

#import vtk
#recorder = vtk.vtkInteractorEventRecorder()
#recorder.SetInteractor(window.getVTKInteractor())
#recorder.SetFileName('record.log')
#recorder.Record()

window.start()
