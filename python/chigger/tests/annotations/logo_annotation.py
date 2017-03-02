#!/usr/bin/env python
from chigger import RenderWindow
from chigger.annotations import ImageAnnotation

# Logos
moose = ImageAnnotation(filename='moose.png', position=[0, 0],
                       horizontal_alignment='left', vertical_alignment='bottom')

marmot = ImageAnnotation(filename='marmot_green.png', position=[0, 1],
                        horizontal_alignment='left', vertical_alignment='top')

pika = ImageAnnotation(filename='pika_white.png', position=[1, 1],
                      horizontal_alignment='right', vertical_alignment='top')


chigger = ImageAnnotation(filename='chigger_white.png', position=[1., 0.],
                         horizontal_alignment='right', vertical_alignment='bottom')
inl = ImageAnnotation(filename='inl.png')

# Create the window
window = RenderWindow(moose, marmot, pika, chigger, inl, test=True)
window.write('logo_annotation.png')
window.start()
