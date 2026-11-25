# ConservativeDiffuseInterfaceVOFMULESCorrector

!syntax description /UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector

## Description

`ConservativeDiffuseInterfaceVOFMULESCorrector` turns the donor (upwind) volume-fraction solve of
a diffuse-interface flow into a bounded, high-resolution, interface-compressing transport with a
MULES (multidimensional universal limiter for explicit solution) flux limiter. After each donor solve of the volume
fraction [!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/variable) in the
system [!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/system_name), it
applies an explicit correction to the donor face fluxes and publishes the face fluxes the volume
fraction actually advanced with. The algorithm is described in detail in the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note.

### Corrected face flux

With the volumetric face flux $\phi_f$ of
[!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/face_flux) and face area
$|S_f|$, the donor flux is $F_f^D = \phi_f |S_f| \alpha_{up}$. The target flux uses a Van Leer
limited face value $\alpha_f^{VL}$ plus an interface compression term:

\begin{equation}
F_f = \phi_f |S_f| \left[ \alpha_f^{VL}
  + c_\alpha \, \mathrm{sign}(\phi_f) \, \alpha_f (1 - \alpha_f) \,
    \hat{\mathbf{n}}_i \cdot \hat{\mathbf{n}}_f \right]_{[0, 1]},
\end{equation}

where $c_\alpha$ is
[!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/compression_factor),
$\alpha_f$ the linear interpolation of the two cell values, $\hat{\mathbf{n}}_i$ the interface
unit normal of [!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/interface_normal),
$\hat{\mathbf{n}}_f$ the face unit normal, and $[\cdot]_{[0,1]}$ a clip to the bounds. The
correction $C_f = F_f - F_f^D$ is applied as

\begin{equation}
F_f^{limited} = F_f^D + w_k \lambda_f C_f,
\qquad 0 \le \lambda_f \le 1,
\end{equation}

with $w_k = 1$ for the first correction sweep and $0.5$ for later ones.
[!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/n_alpha_corrections) sets the
number of sweeps; zero publishes the donor flux unchanged. On boundaries, a closed face carries no
correction, and a fixed-value outflow face carries its boundary value. With
[!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/outflow_two_term_expansion),
an open outflow face carries the linear extrapolation $\alpha_P + \nabla \alpha_P \cdot
\mathbf{d}_{Pf}$, clamped to $[0, 1]$, in place of the cell value. The limiter bounds both, so the
outflow boundary conditions must use the cell value: a two-term expansion in the boundary
condition itself would enter the upwind solve unbounded, and is rejected.

### Limiter

The face limiter $\lambda_f$ keeps every cell within the range of its neighbors' values. Each
cell $P$ has the capacities

\begin{equation}
\psi_P^+ = \frac{V_P}{\Delta t} \max(0, \alpha_{P,max} - \alpha_P),
\qquad
\psi_P^- = \frac{V_P}{\Delta t} \max(0, \alpha_P - \alpha_{P,min}),
\end{equation}

from which cell limiters for incoming and outgoing corrections follow; each face then takes the
smaller of the limiters of the cell it drains and the cell it fills. The cell and face limiters
are tightened together for up to
[!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/n_limiter_iterations) passes,
stopping early once no face limiter changes anywhere. A face limiter may only tighten across the
outer iterations of a time step, which keeps the outer loop from cycling between limiter states.

In parallel, only the process that owns a cell sees all of its faces, so the capacities, sums and
cell limiters are computed for owned cells only. The cell limiters next to a partition boundary
are sent to the neighboring process, so both processes sharing a face accept the same face
limiter and the result does not depend on the partitioning.

### Published fluxes

The corrector publishes two face functors, area-integrated and averaged over the volume-fraction
subcycles of the step:

- `alpha_phi_limited`, the limited volume-fraction flux $F_f^{limited}$;
- `rho_phi`, the mass flux
  $\rho_g \phi_f |S_f| + (\rho_l - \rho_g) F_f^{limited}$ with the phase densities of
  [!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/liquid_density) and
  [!param](/UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector/gas_density).

The momentum equation advects with `rho_phi`, so momentum and mass move with the same flux. The
fluxes of the last accepted time step are restartable, and every attempt at a step starts from
them.

This object is created by
[WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics](WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.md),
which sets its face flux to the `lagged_corrected_face_phi` of the
[ConservativeDiffuseInterfaceRhieChowMassFlux](ConservativeDiffuseInterfaceRhieChowMassFlux.md),
and is applied by the [ReducedPressurePIMPLE](ReducedPressurePIMPLE.md) executioner.

## Example Input Syntax

In this example, the volume-fraction physics `vof` creates this corrector for a slab of liquid
advected at uniform velocity, with one correction sweep, six limiter passes and no interface
compression.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/vof_advection.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated

!syntax parameters /UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector

!syntax inputs /UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector

!syntax children /UserObjects/ConservativeDiffuseInterfaceVOFMULESCorrector
