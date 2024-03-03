# PresetDisplacement

!syntax description /BCs/PresetDisplacement

The PresetDisplacement class takes a displacement time history as input (provided using a function), differentiates it twice using backward Euler method to obtain the corresponding acceleration time history. This acceleration time history is then integrated using Newmark time integration method to obtain a modified displacement time history which is prescribed at the user provided boundary in the direction corresponding to the displacement variable provided as input. This modification in displacement time history ensures that there are no numerical errors in the acceleration or velocity response. 

!syntax parameters /BCs/PresetDisplacement

!syntax inputs /BCs/PresetDisplacement

!syntax children /BCs/PresetDisplacement

!bibtex bibliography
