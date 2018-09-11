# TextAnnotation


The `TextAnnotaion` object is designed to add 2D text objects with the render window. This text
can include multiline text as well as math, as shown in [math-annotation]. As shown in the example,
math is entered by surrounding the text with "$"; VTK uses
[MathText](https://matplotlib.org/users/mathtext.html) syntax.

!listing math_annotation.py
         id=math-annotation
         start=import
         caption=Example of creating a `TextAnnotation` containing math.

!media math_annotation.png
       style=width:30%;margin:auto;
       id=math-annotation-output
       caption=Resulting image from running the `math_annotation.py` script.

!chigger options object=chigger.annotations.TextAnnotation

!chigger keybindings object=chigger.annotations.TextAnnotation

!chigger tests object=chigger.annotations.TextAnnotation
