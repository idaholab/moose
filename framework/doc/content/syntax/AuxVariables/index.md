<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# AuxVariables System

- The auxiliary system's purpose is to allow explicit calculations using nonlinear variables.
- These values can be used by kernels, BCs and material properties.

  - Just couple to them as if they were a nonlinear variable.
  
- They will also come out in the output file... useful for viewing things you don't solve for (e.g. velocity).
- Auxiliary variables currently come in two flavors:

  - Element (constant or higher order monomials)
  - Nodal (linear Lagrange)

- When using element auxiliary variables:

  - You are computing average values per element (constant) or Cholesky solve.
  - You can couple to nonlinear variables and both element and nodal auxiliary variables.

- When using nodal auxiliary variables:

  - You are computing values at nodes.
  - You can +only+ couple to nonlinear variables and other nodal auxiliary variables.

- Auxiliary variables have "old" states just like nonlinear variables.

## Further AuxVariable documentation

!syntax list /AuxVariables objects=True actions=False subsystems=False

!syntax list /AuxVariables objects=False actions=False subsystems=True

!syntax list /AuxVariables objects=False actions=True subsystems=False

