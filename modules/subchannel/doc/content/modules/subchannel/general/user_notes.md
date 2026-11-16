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

The monolithic solver solves the coupled mass flow, pressure, and crossflow equations of a block
together, by default with a field-split preconditioner and no crossflow damping
([Monolithic](subchannel_theory.md#monolithic)). If its solve fails or converges slowly:

- +The outer iteration does not converge, or the enthalpy becomes negative.+ Turn on the crossflow
  damping with [!param](/Problem/TriSubChannel1PhaseProblem/crossflow_pseudo_time_damping). Flow
  blockage cases, square lattices, and some sodium cases need it; the converged solution does not
  depend on it.
- +Choosing the preconditioner.+ The default
  [!param](/Problem/TriSubChannel1PhaseProblem/coupled_preconditioner) `fieldsplit_ilu` scales best
  with the axial and lateral size and with the number of time steps. `lu` factors the coupled
  matrix exactly and needs no retries; it is faster for small cases and converges some that the
  field split does not, such as THORS FFM-3A without damping. A field split retries a coupled
  solve that does not converge with more damping.
- +Coupled solves stagnate with tight tolerances.+ With a field split and tight
  [!param](/Problem/TriSubChannel1PhaseProblem/P_tol) and
  [!param](/Problem/TriSubChannel1PhaseProblem/T_tol), once the crossflow damping has decayed, the
  default FGMRES restart of 30 can discard the Krylov space before it resolves the coupled
  crossflow-pressure modes. Raising
  [!param](/Problem/TriSubChannel1PhaseProblem/coupled_gmres_restart) to 100 to 300 avoids this. A
  longer restart also removes the linear-solve failures that raise the damping, which can
  destabilize the outer iteration of flow blockage cases, so it is not the default.
- +Choosing the crossflow pseudo-time step.+ The crossflow damping is a pseudo-time derivative
  whose initial step is set by
  [!param](/Problem/TriSubChannel1PhaseProblem/crossflow_pseudo_time_step). The converged solution
  does not depend on it; only the cost and the robustness of the iteration do. The default of
  $5\times10^{-4}$ s works for the SCM tests. A larger step needs fewer outer iterations when the
  iteration is well behaved; a smaller one damps more strongly, which can help when the outer
  iteration diverges or reaches its iteration limit, at the cost of more iterations. Because the
  convergence can be sensitive to it, as in flow blockage cases, try values a factor of two to five
  either side of the default rather than a single value.
  [Crossflow damping on EBR-II SHRT-17](crossflow_damping_study.md) reports the iteration counts
  over five orders of magnitude of the step.
- +The outer iteration diverges on a coarse axial mesh.+ With the
  `upwind` or `exponential` [!param](/Problem/TriSubChannel1PhaseProblem/interpolation_scheme),
  the discrete flow equations of a coarse axial mesh can have no solution close to the
  `central_difference` one; the solve then diverges, often with a negative enthalpy or a growing
  pressure error. Refining the axial mesh or using `central_difference` restores a converging solve.
  The EBR-II SHRT-17 test with 20 axial cells and upwind interpolation is one such case; it converges
  with 25 or more cells, or with `central_difference` at 20 cells.
- +Other options+ of the coupled solve can be set on the command line with the `scm_coupled_` PETSc
  prefix, for example `-scm_coupled_ksp_monitor` to follow its convergence.

## The displacement variable

!! Intentional comment to provide extra spacing

The 'displacement' variable is used as a way to model a deformed duct. It's a per subchannel variable that affects the calculation of the geometric parameters (wetted perimeter, flow area) [!cite](kyriakopoulos2024validation) of the subchannels next to the duct.

## Naming/Branding

!! Intentional comment to provide extra spacing

Initially SCM was part of [Pronghorn](https://mooseframework.inl.gov/ncrc/applications/ncrc_root_pronghorn.html). It was called `Pronghorn-Subchannel`, `Pr-Sub`. Once the code became a [MOOSE](https://mooseframework.inl.gov/index.html) module it has been re-branded as SCM.
