#!/usr/bin/env python
import chigger
moose = chigger.annotations.ImageAnnotation(filename='../../../chigger/logos/moose.png', opacity=0.5,
                                            scale=0.5, position=[0.5, 0.75])
window = chigger.RenderWindow(moose, size=[400,400], test=True)
window.write('image_annotation.png')
window.start()
