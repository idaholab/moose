# Adaptivity System

!---

## h-Adaptivity

$h$-adaptivity is a method of automatically refining/coarsening the mesh in regions of high/low
estimated solution error.

Concentrate degrees of freedom (DOFs) where the error is highest, while reducing DOFs where the
solution is already well-captured.

1. Compute a measure of error using an `Indicator` object
1. Mark an element for refinement or coarsening based on the error using a `Marker` object

Mesh adaptivity can be employed in both `Steady` and `Transient` executioners.

!---

## Refinement Patterns

!row!
!col! width=75%
MOOSE employs "self-similar", isotropic refinement patterns: when refining an element is split into
elements of the same type.

- For example, when using Quad4 elements, four "child" elements are created when the element is refined.
- Coarsening happens in reverse, children are deleted and the "parent" element is reactivated.
- The original mesh starts at refinement level 0.
!col-end!

!col width=25%
!media darcy_thermo_mech/adaptivity_pattern.png
!row-end!

!---

## Indicator Objects

`Indicators` report an amount of "error" for each element, built-in `Indicators` include:

`GradientJumpIndicator`\\
Jump in the gradient of a variable across element edges. A good "curvature"
indicator that works well over a wide range of problems.

`FluxJumpIndicator`\\
Similar to `GradientJump`, except that a scalar coefficient (e.g. thermal conductivity) can be
provided to produce a physical "flux" quantity.

`LaplacianJumpIndicator`\\
Jump in the second derivative of a variable. Only useful for $C^1$ shape functions.

`AnalyticIndicator`\\
Computes the difference between the finite element solution and a user-supplied `Function`
representing the analytic solution to the problem.

!---

## Marker Objects

After an `Indicator` has computed the error for each element, a decision to refine or coarsen
elements must be made using a `Marker` object.

`ErrorFractionMarker`\\
Selects elements based on their contribution to the total error.

`ErrorToleranceMaker`\\
Refine if error is greater than a specified value and coarsen if it is less.

`ValueThresholdMarker`\\
Refine if variable value is greater than a specific value and coarsen if it is less.

`UniformMarker`\\
Refine or coarsen all elements.

`BoxMarker`\\
Refine or coarsen inside or outside a given box.

`ComboMarker`\\
Combine several of the above `Markers`.

!---

## Input Syntax

!listing adaptivity/cycles_per_step/cycles_per_step.i block=Adaptivity
