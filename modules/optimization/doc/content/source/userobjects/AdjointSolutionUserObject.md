# AdjointSolutionUserObject

!syntax description /UserObjects/AdjointSolutionUserObject

## Overview

This user-object is meant to be used for transient inverse optimization where the the forward solution is needed to compute the adjoint and subsequent gradient (i.e. nonlinear problems and material inversion).
The object acts similarly to [SolutionUserObject.md] and can be used by objects such as [SolutionAux.md] and [SolutionFunction.md].
The difference stems from the [!param](/UserObjects/AdjointSolutionUserObject/reverse_time_end) parameter where the solution loaded is reversed in time. This is due to the reverse time-stepping required to evaluate the adjoint solution.
Due to specificity of this object's application, only exodus files can be loaded.

## Example Input File Syntax

Here is a material inversion example where the forward and adjoint models are defined as:

!listing materialTransient/optimize_grad.i block=MultiApps

The forward app outputs using [Exodus.md], which results in the file `optimize_grad_out_forward0.e` being generated:

!listing materialTransient/forward.i block=Outputs

The adjoint app reads in this outputted file and a [SolutionAux.md] sets the `u` auxiliary variable to it's values, while reversing the time:

!listing materialTransient/gradient.i block=UserObjects AuxKernels replace=['forward_out.e','optimize_grad_out_forward0.e']

!syntax parameters /UserObjects/AdjointSolutionUserObject

!syntax inputs /UserObjects/AdjointSolutionUserObject

!syntax children /UserObjects/AdjointSolutionUserObject
