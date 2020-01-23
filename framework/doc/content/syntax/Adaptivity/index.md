# Adaptivity System

MOOSE employs $h$-adaptivity to automatically refine or coarsen the mesh in regions of high or low
estimated solution error, respectively. The idea is to concentrate degrees of freedom (DOFs) where
the error is highest, while reducing DOFs where the solution is already well-captured. This is
achieved through splitting and joining elements from the original mesh based on an error
[`Indicator`](/Adaptivity/Indicators/index.md). Once an error has been computed, a
[`Marker`](/Adaptivity/Markers/index.md) is used to decide which elements to refine or coarsen. Mesh
adaptivity can be employed with both `Steady` and `Transient` Executioners.

## Refinement Patterns

MOOSE employs "self-similar", isotropic refinement patterns, as shown in the figure. When an element
is marked for refinement, it is split into elements of the same type. For example, when using Quad4
elements, four "child" elements are created when the element is refined. Coarsening happens in
reverse, children are deleted and the "parent" element is reactivated. The original mesh starts at
refinement level 0. Each time an element is split, the children are assigned a refinement level one
higher than their parents.

!media framework/adaptivity.png
       caption=Self-similar refinement pattern utilized by MOOSE for adaptivity for 1D linear,
               2D quadrilatrel, and 3D hexahedron elements.

## Cycles and Intervals

MOOSE normally performs one adaptivity step per solve. However, developers have the ability to
increase or decrease the amount of adaptivity performed through the "cycles" and "interval" parameters.

The "cycles" parameter can be set to perform multiple adaptivity cycles for a single solve. This is
useful for cases where one would like to resolve a sharp feature in a single step, such as in the case
of an introduced nucleus.

!listing test/tests/mesh/adapt/adapt_test_cycles.i block=Executioner/Adaptivity

The "interval" parameter can be set to decrease the amount of adaptivity is performed so that
it is performed on every _nth_ step. This can sometimes help to speed up your simulation as adaptivity
can be somewhat expensive to perform.

!listing test/tests/mesh/adapt/interval.i block=Executioner/Adaptivity

!syntax parameters /Adaptivity

!syntax list /Adaptivity objects=False actions=False subsystems=True
