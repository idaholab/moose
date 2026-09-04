# IndicatorThresholdCriterion

!syntax description /Remeshing/Criteria/IndicatorThresholdCriterion

## Description

The measured quantity is the largest value of an indicator field over the active elements of the
mesh. The criterion fires when that maximum, reduced over all ranks, exceeds
[!param](/Remeshing/Criteria/IndicatorThresholdCriterion/refine_threshold).

[!param](/Remeshing/Criteria/IndicatorThresholdCriterion/indicator) names a sub-block of
[Indicators](syntax/Adaptivity/Indicators/index.md), which is also the name of the
`CONSTANT MONOMIAL` field that indicator writes; the criterion reads the field by that name. Any
indicator serves, and a [GradientJumpIndicator.md] on the variable whose front is being tracked is
the usual choice. The engine recomputes the indicators on the current mesh before evaluating this
criterion, since the mesh adaptivity that ordinarily drives them is not running.

An `[Adaptivity]` block that defines indicators and nothing else is legal beside `[Remeshing]`; only
mesh-modifying adaptivity is excluded, as described in
[Adaptivity](syntax/Remeshing/index.md#adaptivity). Naming a marker in that block turns adaptivity
into a mesh modifier and ends the run with an error.

### Firing on Over Refinement id=over_refinement

[!param](/Remeshing/Criteria/IndicatorThresholdCriterion/sizing_variable) and
[!param](/Remeshing/Criteria/IndicatorThresholdCriterion/coarsen_fraction) add a second, optional
trigger: the criterion also fires when the diameter of any active element falls below that fraction
of the target size the field holds on it. This is what asks for a coarsening event once the feature
that justified the refinement has moved on, so that a refining remesher and a coarsening one can be
paced by one criterion. The two parameters go together, and supplying either alone is a setup error.

The over-refinement test compares every element against its own target, so there is no single
measured quantity to reduce; the flag itself is reduced instead, and the criterion fires when any
rank raised it.

Both fields are read as one value per element and must therefore be `CONSTANT MONOMIAL` variables.

## Example Input File Syntax

!listing test/tests/remeshing/refine_front.i block=Adaptivity

!listing test/tests/remeshing/refine_front.i block=Remeshing

The over-refinement trigger, paired with a splitting and a coarsening remesher:

!listing test/tests/remeshing/refine_coarsen_cycle.i block=Remeshing/Criteria

!syntax parameters /Remeshing/Criteria/IndicatorThresholdCriterion

!syntax inputs /Remeshing/Criteria/IndicatorThresholdCriterion

!syntax children /Remeshing/Criteria/IndicatorThresholdCriterion
