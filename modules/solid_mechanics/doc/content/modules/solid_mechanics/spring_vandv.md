# Spring elements: verification

Every case of `test/tests/spring/static` is one spring element whose values are checked against hand calculations stated in the header of its input. The deformations and forces below are the ones of the gold files.

## Linear spring

A spring along the global X axis between a held node and a loaded node, stiffnesses (100, 200, 300) along its local axes, loads (10, 4, 3): deformations (0.1, 0.02, 0.01) and the force (10, 4, 3), tension positive. The same element inclined along (3, 0, 4) under the loads rotated to its frame, (3.6, 4, 9.8), gives the global displacements (0.052, 0.02, 0.086) and the same local force. As a ground spring, with node 1 held and node 0 pushed toward it by 10, node 0 moves 0.1 and the force is $-10$, a compression. The Jacobian testers of the linear spring and of the spring with six degrees of freedom (a rotational law with stiffnesses (10, 20, 30) under the moments (1, 0.5, 0.3), rotations (0.1, 0.025, 0.01)) agree with the finite-difference Jacobian to round-off.

!listing modules/solid_mechanics/test/tests/spring/static/linear_spring.i block=Materials

## Spring and dashpot in parallel

A linear spring of stiffness 50 and a dashpot of damping 50 on one element through `base_name`, under a constant load of 5 from the first step: the implicit Euler recursion $u_{n+1} = (u_n + \Delta t \, F / c) / (1 + \Delta t \, k / c)$ with $\Delta t = 0.5$ gives 0.033333, 0.055556, 0.070370 and 0.080247, the spring force and the dashpot force adding up to the load at every step.

## Nonlinear elastic spring

The bilinear backbone of the family's nonlinear cases has the stiffness 100 up to a force of 5 at a deformation of 0.05 and 20 beyond, and is odd. Under a prescribed deformation of 0.3 the force is $5 + 0.25 \times 20 = 10$ (7 at 0.15); a compression-only backbone (stiffness 200, zero force in tension) gives $-4$ at $-0.02$ and a tension-only one (300) gives 3 at 0.01. Under the loads (10, $-4$, 3) on three bilinear axes the deformations are (0.3, $-0.04$, 0.03). Jacobian tester: 1.6e-10.

## Hysteretic spring

The bilinear backbone under a prescribed deformation cycle, 0 to 0.3, to $-0.3$ and back to 0.3 in steps of 0.075:

| Deformation | 0.075 | 0.15 | 0.225 | 0.3 | 0.225 | 0.15 | 0.075 | 0 | -0.075 | -0.15 | -0.225 | -0.3 | -0.225 | -0.15 | -0.075 | 0 | 0.075 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Force | 5.5 | 7 | 8.5 | 10 | 2.5 | -1 | -2.5 | -4 | -5.5 | -7 | -8.5 | -10 | -2.5 | 1 | 2.5 | 4 | 5.5 |

The unloading branch from (0.3, 10) is $10 + 2 F((\delta - 0.3) / 2)$: the elastic unloading spans twice the yield force (zero force at 0.2), and from $-0.05$ on the branch of a bilinear backbone coincides with the backbone, which is the kinematic-hardening loop. The reloading branch from $(-0.3, -10)$ is its mirror image. Under the load history 10, 0, $-5$, $-10$ with the backtracking line search the deformations are 0.3, 0.2, $-0.05$ and $-0.3$, each step in one or two Newton iterations. A reversal inside the loop, 0.3 to 0.2 and up again, starts the branch $2 F((\delta - 0.2) / 2)$ from (0.2, 0), 5 at 0.25 and 10 at 0.3 where it meets the backbone, which gives 11 at 0.35: the law keeps the last reversal only, and this case pins that. The Jacobian tester runs on the backbone; at a reversal point the tangent is discontinuous and a finite-difference Jacobian is not defined.

## Mohr-Coulomb interface spring

Tributary area 2, normal stiffness 1000, shear stiffness 500, cohesion 2, friction angle 30 degrees, under a prescribed closure of 0.01, then a shear deformation of (0.05, 0.02) at that closure, then an opening through zero:

| Stage | Normal force | Shear force 2 | Shear force 3 |
|---|---|---|---|
| closure 0.01 | -20 | 0 | 0 |
| shear (0.0125, 0.005), elastic | -20 | 12.5 | 5 |
| shear (0.05, 0.02), sliding | -20 | 14.435 | 5.774 |
| closure 0.005 during the opening | -10 | 9.0745 | 3.6298 |
| zero closure, the cohesion alone | 0 | 3.7139 | 1.4856 |
| open | 0 | 0 | 0 |

The limit at the closure of 0.01 is $(2 + 10 \tan 30^\circ) \times 2 = 15.547$, reached at a shear deformation of 0.0155; the sliding resultant keeps the direction of the elastic trial, (0.9285, 0.3714). With the closure and the shear deformation along axis 2 prescribed and a load of 3 along axis 3, the resultant stays on the limit: the force along axis 2 is $\sqrt{15.547^2 - 3^2} = 15.2548$. The Jacobian tester compares the hand-coded and the finite-difference Jacobians at the converged state of that sliding step; the ratio sits at the 1e-7 level, the curvature of the radial return seen by the finite differences, and the entries agree to the printed digits.

A bonded interface, unit area, normal and shear stiffness 1000, cohesion 4, friction angle 20 degrees, initial normal stress 1 and tensile strength 2, opened by 0.001 per time unit with a shear deformation of (0.05, 0.02), then closed again by 0.002 with the shear pushed to 0.1 along axis 2:

| Stage | Normal stress | Normal force | Shear force 2 | Shear force 3 |
|---|---|---|---|---|
| opening 0.001, the initial stress relaxed | 0 | 0 | 3.7139 | 1.4856 |
| opening 0.002, half the tensile strength | $-1$ | 1 | 1.8570 | 0.7428 |
| opening 0.003, the tensile strength | $-2$ | 2 | 0 | 0 |
| opening 0.004, separated | | 0 | 0 | 0 |
| closed again, the bond back (default) | 3 | $-2$ | 5.0919 | 0 |
| closed again, permanent breakage, residual cohesion 1 and friction angle 10 degrees | 3 | $-2$ | 1.5290 | 0 |

The shear limit falls from the cohesion at zero normal stress to zero at the tensile strength, $4 \, (1 - 1/2) = 2$ half way, in the direction (0.9285, 0.3714) of the trial; the normal force is the tensile part of the normal stress. The slip took up the whole shear while the interface was open, so the closed interface slides along axis 2 alone, on $4 + 3 \tan 20^\circ$ when the bond is back and on $1 + 3 \tan 10^\circ$ after a permanent breakage. The Jacobian tester runs on the tension state half way to the tensile strength, where the shear couples to the normal deformation through the slope $c / \sigma_t$ of the falling limit.

With an initial normal stress of 10 and no closure the normal force stays zero while the shear capacity is the same 15.547 from the start, the forces (14.435, 5.774) at the same shear deformation; an opening of 0.005 relaxes the limit to $(2 + 5 \tan 30^\circ) \times 2 = 9.7735$, forces (9.0745, 3.6298), at 0.01 the limit is the cohesion alone, 4, and an opening beyond it separates the interface. With the cohesion alone, no initial stress and no closure, the capacity at rest is 4, forces (3.7139, 1.4856) at the same shear deformation, lost at the first opening. The Jacobian tester at the sliding state under the initial stress with an opening checks the coupling of the shear to the normal deformation while the normal stiffness is zero.

## Viscous spring

Damping (50, 0, 0): under a prescribed velocity of 0.1 the force is 5 at every step of the implicit Euler integration, and under a constant load of 5 the node moves at 0.1 per unit time from the first step. Jacobian tester: 2.4e-10.

!bibtex bibliography
