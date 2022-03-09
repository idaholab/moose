# MortarFrictionalPressureVectorAux

## Description

The `MortarFrictionalPressureVectorAux` outputs the frictional pressure vector components using information from the mortar contact solution and its
geometry. Nodal information is used to associate the two frictional Lagrange's multiplier vectors in three-dimensions with their corresponding
tangent vectors generated at the time the mortar segment mesh is generated. This auxiliary kernel is automatically included in the `ContactAction` when the user selects three-dimensional, frictional mortar contact.
The computation of the frictional pressure vector can be expressed as:

\begin{equation*}
  \boldsymbol{p}_{\mu} = \lambda_{fx} \cdot \boldsymbol{t}_{x} + \lambda_{fy} \cdot \boldsymbol{t}_{y},
\end{equation*}

where $\boldsymbol{p}_{\mu}$ denotes the frictional pressure vector, $\lambda_{fx}$ and $\lambda_{fx}$ are two frictional Lagrange's multipliers, and $\boldsymbol{t}_{x}$ and $\boldsymbol{t}_{y}$ are two mutually perpendicular tangent vectors. Note that the directions $x$ and $y$ need not be aligned with global axes. This object is an [AuxKernel](AuxKernels/index.md), and is used only for the purpose of output. Note that the [Contact](Contact/index.md) action sets this object up automatically, so it is typically not necessary to include this in an input file.

## Input example

Creation of auxiliary variables, i.e. frictional pressure vector components:

!listing test/tests/3d-mortar-contact/frictional-mortar-3d.i block=AuxVariables

Creation of component-wise auxiliary kernels:

!listing test/tests/3d-mortar-contact/frictional-mortar-3d.i block=AuxKernels

!syntax parameters /AuxKernels/MortarFrictionalPressureVectorAux

!syntax inputs /AuxKernels/MortarFrictionalPressureVectorAux

!syntax children /AuxKernels/MortarFrictionalPressureVectorAux
