# Large-Deformation J2 Plasticity, Force Control

## Description

The force-controlled companion of the
[displacement-controlled plastic example](modules/contact/rigid_contact/examples/ld_inelastic_disp.md).
Same quarter-sphere J2 plasticity body, same mesh + `sphere_refinement`
knob, pressed by an analytic
[SphereContactor.md] --- but now the indenter's $y$-translation is a
scalar unknown driven by the load-control layer: a target reaction
$F(t)$ is prescribed, and the outer Newton solves for the depth
consistent with that reaction.

The applied force ramps linearly from 0 to $1.5\times 10^5$ over
$t \in [0, 1]$, a level that pushes the plastic zone well outside the
initial contact patch by the end of the run.

## Contact setup

The [`[RigidContact]`](syntax/RigidContact/index.md) sub-block adds the
force-control extensions on top of the analytic-level-set stack.
Setting [!param](/RigidContact/RigidContactAction/force) to a function
name activates:

- A [NodalArea](/userobjects/NodalArea.md) UO on the contact face
  (weights $w_i$ for the reaction sum).
- A scalar variable for the indenter's translation along
  [!param](/RigidContact/RigidContactAction/load_direction) (default
  name `indenter_<axis>`), with a small positive initial condition to
  give Newton some LM to bite on immediately.
- A [RigidBodyLoadControl.md] scalar kernel that enforces
  $R_s = \sum_i w_i \lambda_i (n_i \cdot \hat{d}) - F(t) = 0$.

!listing modules/contact/examples/rigid/ld-inelastic-force/hertz_inelastic_finite_3d_force.i block=RigidContact

The SphereContactor is wired up with `disp_y_scalar = indenter_y` so
that the load-control scalar drives the same translation the reaction
constraint measures:

!listing modules/contact/examples/rigid/ld-inelastic-force/hertz_inelastic_finite_3d_force.i block=UserObjects/sphere

## Solver

The [Predictor](/Executioner/Predictor/index.md) block emits a
[RigidBodyContactPredictor.md] that resolves the local
$(u_{\text{contact}}, \lambda, s)$ subproblem before the full Newton
fires.  Without it, cumulative nonlinear iterations grow rapidly under
the load-control coupling; the predictor cuts them by roughly a factor
of three to four on this problem.

!listing modules/contact/examples/rigid/ld-inelastic-force/hertz_inelastic_finite_3d_force.i block=Executioner/Predictor

The rest of the executioner is a stock `Transient` with
`SNESVINEWTONSSLS` + `basic` line search, matching the displacement
case's inner solve.

## Full input

!listing modules/contact/examples/rigid/ld-inelastic-force/hertz_inelastic_finite_3d_force.i

## Results

!media media/contact/rigid_contact/ld_inelastic_force.png
       id=fig:ld_inelastic_force
       caption=Force-controlled J2 plasticity results.  Left:
               `plastic_strain_mag` on the deformed quarter-hemisphere;
               the plastic zone (peak $\sim 5.7\times 10^{-2}$, colored
               fringe) is markedly larger than in the
               displacement-controlled companion because the prescribed
               reaction ramp drives the indenter well past first yield.
               Right: `contactor_force` (the load-control scalar's
               target) plotted against `contactor_displacement` (the
               indenter's advance along the load axis).  The relation
               is quasi-linear once past initial yield, reflecting
               steady hardening at the growing plastic patch.  Run at
               `sphere_refinement = 5` (finer than the shipped default
               of 3).
