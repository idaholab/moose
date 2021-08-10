# NSFVHeatFluxBC

!syntax description /FVBCs/NSFVHeatFluxBC

# Description

Boundary condition splitting a constant heat flux among a two-phase fluid and solid domain
based on a number of different models. There is no general consenus on the best boundary
condition for all flows, and virtually all models for splitting a constant heat flux
among multiple phases are quite crude [!citep](alazmi).

By setting `phase = fluid`, this boundary condition specifies the heat flux in the
fluid energy conservation equation as

\begin{equation}
-\int_\Gamma\kappa_f\cdot\nabla T_f\cdot\hat{n}d\Gamma\ ,
\end{equation}

where

\begin{equation}
-\kappa_f\cdot\nabla T_f\cdot\hat{n}=\frac{\zeta_f}{\zeta_s+\zeta_f}\tilde{q}\ ,
\end{equation}

where $\tilde{q}$ is the total heat flux specified by the `value` parameter,
$\zeta_f$ is the fluid-phase value of $\zeta$ and $\zeta_s$ is the solid-phase value
of $\zeta$. $\zeta$ is a placeholder to be discussed shortly.

By setting `phase = solid`, this boundary condition specifies the heat flux in the
solid energy conservation equation as

\begin{equation}
-\int_\Gamma\kappa_s\cdot\nabla T_s\cdot\hat{n}d\Gamma\ ,
\end{equation}

where

\begin{equation}
-\kappa_s\cdot\nabla T_s\cdot\hat{n}=\frac{\zeta_s}{\zeta_s+\zeta_f}\tilde{q}\ .
\end{equation}

The `splitting` parameter determines the form of $\zeta$; the heat flux split can
be assigned based on

- porosity, in which case $\zeta_f=\epsilon$ and $\zeta_s=1-\epsilon$
- effective thermal conductivity, in which case $\zeta_f=\kappa_f$ and $\zeta_s=\kappa_s$
- thermal conductivity, in which case $\zeta_f=k_f$ and $\zeta_s=k_s$

$\zeta$ may also be calculated either from the local value at the wall by setting
`locality = local` or from a domain-averaging postprocessor by setting
`locality = global`. For instance, it is well known that the porosity at the wall is unity
due to point contacts between pebbles and the wall; if splitting the heat flux based on the
porosity, the unity porosity would result in the entire heat flux depositing in the
fluid phase, when in reality the combined conduction, convection, and radiation processes
always result in heat deposition into both phases. The `locality` parameter attempts to
capture some of this phasic behavior independent of the closures precisely at the wall.

This boundary condition does not specify the
interpretation of what constitutes the domain, so you may choose for the global values to
represent averages over the entire geometry, or possibly a finite-width region near the wall.

For the following combinations of splitting method and locality, the following parameters
must be provided:
- `splitting = porosity`, `locality = local`: `porosity` coupled variable
- `splitting = porosity`, `locality = global`: `average_porosity` postprocessor
- `splitting = thermal_conductivity`, `locality = local`: nothing, material property grabs don't require user input
- `splitting = thermal_conductivity`, `locality = global`: `average_k_fluid` and `average_k_solid` postprocessors
- `splitting = effective_thermal_conductivity`, `locality = local`: `porosity` coupled variable due to internal
   representation of $\kappa_f$ as $\epsilon\tilde{\kappa}_f$, where $\tilde{\kappa_f}\equiv\frac{\kappa_f}{\epsilon}$.
- `splitting = effective_thermal_conductivity`, `locality = global`: `average_kappa` and `average_kappa_solid` postprocessors,
   as well as `average_eps` postprocessor due to internal representation of $\kappa_f$ as $\epsilon\tilde{\kappa}_f$.

! alert note
To protect against cases where at the first time step
the thermal conductivity or effective thermal conductivity might not have yet
been initialized, or cases where the coupled postprocessors have not yet been
evaluated, we use an equal flux splitting.

!syntax parameters /FVBCs/NSFVHeatFluxBC

!syntax inputs /FVBCs/NSFVHeatFluxBC

!syntax children /FVBCs/NSFVHeatFluxBC

!bibtex bibliography
