# Bond-based Peridynamic Heat Source Kernel

## Description

The `HeatSourceBPD` Kernel considers the heat source as a directly contribution to residual calculation of heat conduction models. Since the kernel iterate on each element but the heat source resides at each discrete material point, the repetition of a material point in a element-wise loop should be considered. And this is achieved by dividing the residual for each element by the total number of element connected at a material point.

!syntax parameters /Kernels/HeatSourceBPD

!syntax inputs /Kernels/HeatSourceBPD

!syntax children /Kernels/HeatSourceBPD
