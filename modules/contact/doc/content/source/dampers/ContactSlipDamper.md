# ContactSlipDamper

!syntax description /Dampers/ContactSlipDamper

## Description

This damper minimizes the oscillations that are inherent in the solution of frictional contact problems by limiting the change in contact state from one nonlinear iteration to the next. This is designed specifically to work with node/face mechanical contact enforced using [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md).

This damper specifically addresses reversals in the slip direction from one nonlinear iteration to the next. If a node that is in contact slips farther than it should during a given nonlinear iteration, the resulting residual will cause that node to slip back in the opposite direction in the next nonlinear iteration. If that slip in the opposite direction in this next nonlinear iteration is large enough that it causes the node to slip past the original contact point, it can be extremely difficult to obtain a converged solution because the node keeps slipping back and forth past that original contact point during each nonlinear iteration.

This damper mitigates this problem by scaling back the nonlinear solution update to limit the amount of slip experienced by nodes that are slipping in a direction opposite to the slip direction in the previous nonlinear iteration. For each node in this situation, a scale factor that would bring the node back to its original position is computed, and the minimum value for all such nodes is taken as the overall scale factor. For a simulation with many nodes in contact, this can result initially in small damping factors, but over the course of iterations as the converged solution is approached, the damping factor will typically increase.

The following optional parameters can be used to adjust the behavior of this damper:

- `min_damping_factor` sets a minimum value for the damping. Setting this to a number larger than 0 can permit some amount of excess slip reversals to avoid having the solution getting stuck with extremely small damping factors.
- `max_iterative_slip` allows the damper to also limit the amount of slip that can occur during a nonlinear iteration, in addition to limiting slip reversals. This is prescribed in the length units of the model.
- `damping_threshold_factor` affects the slip reversal limiting behavior of this damper when the magnitude of the slip is small. If the amount of slip is small, the node is likely actually in a sticking state, and simply moving within the limits permitted by the penalty factor. This factor specifies the threshold slip, computed as a multiplier on the limit imposed by the penalty factor, at which the damper will start to limit slip reversals. Setting it to larger numbers will make this damper have less effect on nodes will small amounts of slip.

In addition to limiting the nonlinear solution step, this damper also prints out some useful diagnostic information to the output stream, including statistics on the numbers of nodes in various contact states.

Finally, it should be noted that although this damper is primarily intended to be used with frictional contact, it can be useful for frictionless contact because of its ability to limit the maximum amount of slip.

## Example Input Syntax

!listing test/tests/frictional/sliding_elastic_blocks_2d/sliding_elastic_blocks_2d.i block=Dampers

!syntax parameters /Dampers/ContactSlipDamper

!syntax inputs /Dampers/ContactSlipDamper

!syntax children /Dampers/ContactSlipDamper
