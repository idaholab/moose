# SolutionAux

This AuxKernel works in conjunction with a [SolutionUserObject](/SolutionUserObject.md) to produce fields. If the solution was created on an equivalent mesh, the direct option can be used to read using DOF information. Otherwise, a point locator is used to find the nearest node or element and the corresponding value. Finally, these values can be scaled and/or added by utilizing the *scale_factor* or *add_factor* parameters.

!syntax description /AuxKernels/SolutionAux

!syntax parameters /AuxKernels/SolutionAux

!syntax inputs /AuxKernels/SolutionAux

!syntax children /AuxKernels/SolutionAux
