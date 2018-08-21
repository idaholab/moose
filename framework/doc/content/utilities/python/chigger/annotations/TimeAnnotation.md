# TimeAnnotation


The `TimeAnnotaion` object is an extension of the [TextAnnotation.md] that creates time annotations
to convert time in seconds to other formats such as hours, minutes, and seconds.

!listing time_annotation.py
         id=time-annotation
         start=import
         caption=Example of creating a `TimeAnnotation` containing math.

!media time_annotation.png
       style=width:30%;margin:auto;
       id=time-annotation-output
       caption=Resulting image from running the `time_annotation.py` script.

!chigger options object=chigger.annotations.TimeAnnotation

!chigger keybindings object=chigger.annotations.TimeAnnotation

!chigger tests object=chigger.annotations.TimeAnnotation
