# SCM User Notes

The following page is presenting various notes concerning SCM functionality
that might be helpful to both new and seasoned users. This page will be updated
periodically, so check back often!

## SCM index notation

!! Intentional comment to provide extra spacing

SCM follows the index notation presented in the two figures bellow: where in black numbers is the subchannel
index, in white numbers the fuel-pin index and in red numbers the gap index. For both meshes (square, hexagonal) the center of the mesh is the origin.

### Square lattice

!! Intentional comment to provide extra spacing

!media subchannel/getting_started/square_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=sindex
    caption=Square Lattice subchannel index notation, in a $5 \times 5$ sub-assembly.

### Triangular lattice

!! Intentional comment to provide extra spacing

!media subchannel/getting_started/hex_index.png
    style=width:60%;margin-bottom:2%;margin:auto;
    id=hindex
    caption=Triangular Lattice subchannel index notation, in a $3$ ring sub-assemly.

In order for the user to print the index notation for subchannels and pins in a triangualar lattice arrangement, they are invited to run the following Python script:

!listing /scripts/index.py

## Pressure boundary condition caveat

!! Intentional comment to provide extra spacing

SCM's solver is designed to solve for relative pressure: $P_{relative} = P_{absolute} - P_{boundary}$, such that the relative pressure at the outlet boundary is always zero. The pressure boundary condition is only explicitly used in the calculation of fluid properties. The user should be careful to initialize/set the pressure at the assembly outlet to zero (not setting it at all will also define the default value of zero).

## Troubleshooting failed solves id=troubleshooting

!! Intentional comment to provide extra spacing

### Monolithic solver

!! Intentional comment to provide extra spacing

The monolithic solver solves the coupled mass flow, pressure, and crossflow equations of a block
together, by default with a field-split preconditioner and crossflow damping
([Monolithic](subchannel_theory.md#monolithic)). If its solve fails or converges slowly:

- +Choosing the preconditioner.+ The default
  [!param](/Problem/TriSubChannel1PhaseProblem/monolithic_preconditioner) `fieldsplit_ilu` scales
  best with the axial and lateral size and with the number of time steps. `lu` factors the monolithic
  matrix exactly and needs no retries, which makes it faster for small cases, but its cost grows
  faster than the system size. A field split retries a monolithic solve that does not converge with
  more damping.
- +Turning off the damping.+ The crossflow damping vanishes at convergence and does not change the
  converged solution. It can be turned off with
  [!param](/Problem/TriSubChannel1PhaseProblem/crossflow_pseudo_time_damping). Without it, the
  crossflow block of a steady solve that starts from zero crossflow can have a zero diagonal, where
  its time and friction terms vanish and its advection terms cancel, which a field split with ILU(0)
  cannot factor; `lu` does not have this limitation.
- +Monolithic solves stagnate with tight tolerances.+ With a field split and tight
  [!param](/Problem/TriSubChannel1PhaseProblem/P_tol) and
  [!param](/Problem/TriSubChannel1PhaseProblem/T_tol), once the crossflow damping has decayed, the
  default FGMRES restart of 30 can discard the Krylov space before it resolves the coupled
  crossflow-pressure modes. A longer
  [!param](/Problem/TriSubChannel1PhaseProblem/monolithic_gmres_restart) avoids this. It also removes
  the linear-solve failures that raise the damping, which can destabilize the outer iteration, so it
  is not the default.
- +Choosing the crossflow pseudo-time step.+ The initial step of the damping is set by
  [!param](/Problem/TriSubChannel1PhaseProblem/crossflow_pseudo_time_step). The converged solution
  does not depend on it; only the cost and the robustness of the iteration do. A smaller step damps
  more strongly, at the cost of more iterations. [The crossflow damping study](crossflow_damping_study.md)
  shows how the iteration count depends on it.
- +The outer iteration diverges on a coarse axial mesh.+ With the
  `upwind` or `exponential` [!param](/Problem/TriSubChannel1PhaseProblem/interpolation_scheme),
  the discrete flow equations of a coarse axial mesh can have no solution close to the
  `central_difference` one; the solve then diverges, often with a negative enthalpy or a growing
  pressure error. Refining the axial mesh or using `central_difference` restores a converging solve.
- +Other options+ of the monolithic solve can be passed as PETSc options prefixed with
  `-scm_monolithic_`, for example `-scm_monolithic_ksp_monitor` to follow its convergence.

## The displacement variable

!! Intentional comment to provide extra spacing

The 'displacement' variable is used as a way to model a deformed duct. It's a per subchannel variable that affects the calculation of the geometric parameters (wetted perimeter, flow area) [!cite](kyriakopoulos2024validation) of the subchannels next to the duct.

## Naming/Branding

!! Intentional comment to provide extra spacing

Initially SCM was part of [Pronghorn](https://mooseframework.inl.gov/ncrc/applications/ncrc_root_pronghorn.html). It was called `Pronghorn-Subchannel`, `Pr-Sub`. Once the code became a [MOOSE](https://mooseframework.inl.gov/index.html) module it has been re-branded as SCM.
