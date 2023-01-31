# Transport Systems

!---

# Transport Systems

!row!
!col! width=69%
!style! fontsize=90%
- Custom action that:

  - Adds angular/scalar flux variables
  - Adds kernels for these variables
  - Adds boundary conditions for these variables

- Allows to select:

  - Particle types: `particle = neutron/photon/phonon/thermal/common`
  - `equation_type = transient/eigenvalue/steady-state`
  - List of all reflecting boundaries: `ReflectingBoundary`
  - List of all vacuum boundaries: `VacuumBoundary`
  - Number of energy groups: `G`

- TransportSystems specifies discretization schemes in sub-blocks
!style-end!
!col-end!

!col! width=1%
!!
!col-end!

!col! width=30%
```
[TransportSystems]
  particle = neutron
  equation_type = eigenvalue

  ReflectingBoundary = 'left right bottom top'
  VacuumBoundary = 'front back'

  G = 2

  [diff]
    scheme = CFEM-Diffusion
    n_delay_groups = 8
    ...
  []
[]
```
!col-end!
!row-end!

!---

# DFEM-SN Scheme

!row!
!col! width=69%
!style! fontsize=90%
- Finite element family:

  - `family = MONOMIAL/L2_LAGRANGE`
  - `order = FIRST`

- Angular quadrature (`AQtype`):

  - `Gauss-Chebyshev`: `NPolar`, `NAzmthl`
  - `Level-Symmetric`: `AQorder`
  - `Bickley3-Optimized`: `NPolar`, `NAzmthl`

- Typical parameters:

  - `using_array_variable=true`: Improves memory efficiency
  - `hide_angular_flux=true`: Removes angular flux from output
  - `hide_higher_flux_moment=0`: Removes flux moments from output (only scalar flux)
!style-end!
!col-end!

!col! width=1%
!!
!col-end!

!col! width=30%
```
[TransportSystems]
  particle = neutron
  equation_type = eigenvalue

  ReflectingBoundary = 'left right bottom top'
  VacuumBoundary = 'front back'

  G = 2

  [diff]
    scheme = DFEM-SN
    n_delay_groups = 8

    family = L2_LAGRANGE
    order = FIRST
    AQtype = Gauss-Chebyshev
    NPolar = 2
    NAzmthl = 3

    using_array_variable = true
    hide_angular_flux = true
  []
[]
```
!col-end!
!row-end!

!---

# CFEM-DIFFUSION Scheme

!row!
!col! width=69%
- Finite element family:

  - `family = LAGRANGE`
  - `order = FIRST/SECOND`

- Ensuring full Jacobian is assembled:

  - `assemble_scattering_jacobian = true`
  - `assemble_fission_jacobian = true`
  - `assemble_delay_jacobian = true`
!col-end!

!col! width=1%
!!
!col-end!

!col! width=30%
```
[TransportSystems]
  particle = neutron
  equation_type = eigenvalue

  ReflectingBoundary = 'left right bottom top'
  VacuumBoundary = 'front back'

  G = 2

  [diff]
    scheme = CFEM-Diffusion
    n_delay_groups = 8

    assemble_scattering_jacobian = true
    assemble_fission_jacobian = true
    assemble_delay_jacobian = true
  []
[]
```
!col-end!
!row-end!
