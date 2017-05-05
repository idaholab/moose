# Adaptivity System
!parameters /Adaptivity

## h-Adaptivity

!media docs/media/element_adaptivity.png width=250px float=right padding-left=20px caption=Self-similar refinement pattern utilized by MOOSE for adaptivity.

MOOSE employs $h$-adaptivity to automatically refine or coarsen the mesh in regions of high or low estimated
solution error, respectively. The idea is to concentrate degrees of freedom (DOFs) where the error is highest,
while reducing DOFs where the solution is already well-captured. This is achieved through splitting and joining
elements from the original mesh based on an error [`Indicator`](/Adaptivity/Indicators/index.md). Once an error
has been computed, a [`Marker`](/Adaptivity/Markers/index.md) is used to decide which elements to refine or coarsen.
Mesh adaptivity can be employed in both `Steady` and `Transient` Executioners.

## Refinement Patterns


MOOSE employs "self-similar", isotropic refinement patterns, as shown in the figure. When an element is marked
for refinement, it is split into elements of the same type. For example, when using Quad4 elements, four "child"
elements are created when the element is refined. Coarsening happens in reverse, children are deleted and the
"parent" element is reactivated. The original mesh starts at refinement level 0. Each time an element is split, the
children are assigned a refinement level one higher than their parents.

!subsystems /Adaptivity
