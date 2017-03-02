#!/usr/bin/env python
import chigger
text = chigger.annotations.TextAnnotation(text='This is a test.', font_size=14, text_color=[1,0,1], text_opacity=0.5)
window = chigger.RenderWindow(text, size=[300,300], test=True)
window.write('text_annotation.png')
window.start()
