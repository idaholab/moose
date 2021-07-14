# FVMatPropTimeKernel

!syntax description /FVKernels/FVMatPropTimeKernel

## Overview

This object simply populates the residual with the value of the material
property passed in (specified by `mat_prop_time_derivative`). It is the
responsibility of the material providing this property to ensure that the time
derivative is computed properly.

!syntax parameters /FVKernels/FVMatPropTimeKernel

!syntax inputs /FVKernels/FVMatPropTimeKernel

!syntax children /FVKernels/FVMatPropTimeKernel
