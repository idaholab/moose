#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import vtk
import chigger
import mooseutils

#!/usr/bin/env python3
import os
import argparse
import vtk
import chigger

FONTSIZE = 36
PREFIX = 'step10'

def frames():
    """Render frames"""
    camera = vtk.vtkCamera()
    camera.SetViewUp(0.0000000000, 0.9999999979, 0.0000646418)
    camera.SetPosition(0.1801966629, -0.0310247580, 0.1288860739)
    camera.SetFocalPoint(0.1801966629, -0.0310164265, 0.0000000000)

    master_reader = chigger.exodus.ExodusReader('step10_out.e', displacement_magnitude=500)

    temp_rot = chigger.filters.TransformFilter(rotate=[0,0,270])
    temp = chigger.exodus.ExodusResult(master_reader, camera=camera,
                                       variable='temperature',
                                       viewport=[0,2/3.,1,1],
                                       range=[300, 350],
                                       filters=[temp_rot],
                                       cmap='plasma')

    sub_camera = vtk.vtkCamera()
    sub_camera.SetViewUp(0.0000000000, 1.0000000000, 0.0000000000)
    sub_camera.SetPosition(0.0500000000, 0.0500000000, 0.2165474516)
    sub_camera.SetFocalPoint(0.0500000000, 0.0500000000, 0.0000000000)

    sub0_reader = chigger.exodus.ExodusReader('step10_out_micro0.e', timestep=0)
    sub0_result = chigger.exodus.ExodusResult(sub0_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[0,1/3.,1/6.,2/3.])

    sub1_reader = chigger.exodus.ExodusReader('step10_out_micro1.e', timestep=0)
    sub1_result = chigger.exodus.ExodusResult(sub1_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[1/6.,1/3.,2/6.,2/3.])

    sub2_reader = chigger.exodus.ExodusReader('step10_out_micro2.e', timestep=0)
    sub2_result = chigger.exodus.ExodusResult(sub2_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[2/6.,1/3.,3/6.,2/3.])

    sub3_reader = chigger.exodus.ExodusReader('step10_out_micro3.e', timestep=0)
    sub3_result = chigger.exodus.ExodusResult(sub3_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[3/6.,1/3.,4/6.,2/3.])

    sub4_reader = chigger.exodus.ExodusReader('step10_out_micro4.e', timestep=0)
    sub4_result = chigger.exodus.ExodusResult(sub4_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[4/6.,1/3.,5/6.,2/3.])

    sub5_reader = chigger.exodus.ExodusReader('step10_out_micro5.e', timestep=0)
    sub5_result = chigger.exodus.ExodusResult(sub5_reader, variable='phi', cmap='plasma', camera=sub_camera,
                                              viewport=[5/6.,1/3.,6/6.,2/3.])

    subs = [sub0_result, sub1_result, sub2_result, sub3_result, sub4_result, sub5_result]

    cbar = chigger.exodus.ExodusColorBar(temp, sub0_result,
                                         viewport=[0,0,1,1],
                                         width=0.05,
                                         length=0.6,
                                         colorbar_origin=[0.2, 0.725],
                                         location='top')
    cbar.setOptions('primary', title='Temperature (K)', font_size=FONTSIZE, font_color=[0,0,0], num_ticks=6)
    cbar.setOptions('secondary', title='Phase (0=water; 1=steel)', font_size=FONTSIZE, font_color=[0,0,0], num_ticks=3)

    time = chigger.annotations.TextAnnotation(position=[0.1,0.725], font_size=FONTSIZE, text_color=[0,0,0],
                                              justification='center', vertical_alignment='middle')

    tdisp = chigger.annotations.TextAnnotation(position=[0.92,0.825], font_size=FONTSIZE*.75, text_color=[0,0,0],
                                               text='500x Displacement', justification='center', vertical_alignment='middle')


    line0 = chigger.graphs.Line(width=3, label='Sub0')
    line1 = chigger.graphs.Line(width=3, label='Sub1')
    line2 = chigger.graphs.Line(width=3, label='Sub2')
    line3 = chigger.graphs.Line(width=3, label='Sub3')
    line4 = chigger.graphs.Line(width=3, label='Sub4')
    line5 = chigger.graphs.Line(width=3, label='Sub5')

    # VTK messes up the order, that is why the strange input
    graph = chigger.graphs.Graph(line3, line0, line2, line4, line5, line1, color_scheme='BREWER_QUALITATIVE_DARK2',
                                 viewport=[0,0,1,1/3.])

    graph.setOptions('xaxis', title='Time (s)', lim=[0,80], font_color=[0,0,0], font_size=FONTSIZE, num_ticks=9)
    graph.setOptions('yaxis', title='k (W/mK)', lim=[0.5, 12.5], font_color=[0,0,0], font_size=FONTSIZE, num_ticks=5)
    graph.setOptions('legend', label_color=[0,0,0], label_font_size=0.75*FONTSIZE, opacity=1, background=[1,1,1], border=True, border_width=1, border_color=[0,0,0])

    laby = 1/3. - 1/64.
    lab0 = chigger.annotations.TextAnnotation(text='Sub0: x=0', position=[1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')
    lab1 = chigger.annotations.TextAnnotation(text='Sub1: x=0.0608', position=[1/6. + 1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')
    lab2 = chigger.annotations.TextAnnotation(text='Sub2: x=0.1216', position=[2/6. + 1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')
    lab3 = chigger.annotations.TextAnnotation(text='Sub3: x=0.1824', position=[3/6. + 1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')
    lab4 = chigger.annotations.TextAnnotation(text='Sub4: x=0.2432', position=[4/6. + 1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')
    lab5 = chigger.annotations.TextAnnotation(text='Sub5: x=0.304', position=[5/6. + 1/12., laby], font_size=FONTSIZE, text_color=[0.5]*3, justification='center', vertical_alignment='top')

    subs += [lab0, lab1, lab2, lab3, lab4, lab5]
    window = chigger.RenderWindow(temp, cbar, time, tdisp, graph, *subs, size=[1920, 1080],
                                  background=[1,1,1], motion_factor=0.2)

    for i, t in enumerate(master_reader.getTimes()):
        master_reader.setOptions(timestep=i)
        if i > 11:
            sub0_reader.setOptions(timestep=i-11)
            sub1_reader.setOptions(timestep=i-11)
            sub2_reader.setOptions(timestep=i-11)
            sub3_reader.setOptions(timestep=i-11)
            sub4_reader.setOptions(timestep=i-11)
            sub5_reader.setOptions(timestep=i-11)

            line0.setOptions(y=[sub0_reader.getGlobalData('k_eff')], x=[t])
            line1.setOptions(y=[sub1_reader.getGlobalData('k_eff')], x=[t])
            line2.setOptions(y=[sub2_reader.getGlobalData('k_eff')], x=[t])
            line3.setOptions(y=[sub3_reader.getGlobalData('k_eff')], x=[t])
            line4.setOptions(y=[sub4_reader.getGlobalData('k_eff')], x=[t])
            line5.setOptions(y=[sub5_reader.getGlobalData('k_eff')], x=[t])

        time.setOptions(text='Time = {:.2f} sec.'.format(t))
        filename = 'output/{}_{:05d}.png'.format(PREFIX, i)
        window.write(filename)

    window.start()

def movie():
    chigger.utils.img2mov('output/{}_*.png'.format(PREFIX), '{}_result.mp4'.format(PREFIX),
                          duration=30, num_threads=6)
if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    frames()
    movie()
