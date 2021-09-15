# ElementQualityAux

!syntax description /AuxKernels/ElementQualityAux

# Description

Computes per-element quality metrics and puts the value in a field.  An example is shown below using the `SHAPE` metric on
an unstructured mesh.  In this case "perfectly" shaped elements have a value of `1.0` and as the quality degrades so does
the value of the shape metric.

!media media/framework/auxkernels/element_quality.png style=width:60%


The `SHAPE` metric is a good one to use for quads and tris.  You can see the list of all possible metrics below in the documentation for the `metric` parameter.

!syntax parameters /AuxKernels/ElementQualityAux

!syntax inputs /AuxKernels/ElementQualityAux

!syntax children /AuxKernels/ElementQualityAux

!bibtex bibliography
