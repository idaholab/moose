# SurrogateModelAuxKernel

!syntax description /AuxKernels/SurrogateModelAuxKernel

## Overview

This aux-kernel sets the value of an auxiliary variable by evaluating a [surrogate model](Surrogates/index.md). The parameter values used during the evaluation are specified using [!param](/AuxKernels/SurrogateModelAuxKernel/parameters). These values can be constant numbers, [post-processors](Postprocessors/index.md), [functions](Functions/index.md), or field variables. Post-processor and function values must be indicated by listing them in [!param](/AuxKernels/SurrogateModelAuxKernel/scalar_parameters). Variable values should be listed in [!param](/AuxKernels/SurrogateModelAuxKernel/coupled_variables).

!syntax parameters /AuxKernels/SurrogateModelAuxKernel

!syntax inputs /AuxKernels/SurrogateModelAuxKernel

!syntax children /AuxKernels/SurrogateModelAuxKernel
