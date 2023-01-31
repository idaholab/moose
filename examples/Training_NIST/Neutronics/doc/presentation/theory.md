# Theory

!---

# Neutron Transport Equation

!equation
\underbrace{ \frac{1}{v}\frac{\partial \psi}{\partial t} }_{\text{Time derivative}} + \underbrace{ \vec{\Omega}\cdot\vec{\nabla}\psi }_{\text{Streaming}}+ \underbrace{ \Sigma_t\psi }_{\text{Collision}} = \underbrace{ \int_{0}^{\infty}\int_{4\pi}\Sigma_s(\vec{\Omega}'\cdot\vec{\Omega},E'\rightarrow E)\psi(\vec{\Omega}',E')d\Omega'dE' }_{\text{Scattering}} +

!equation
\underbrace{ \frac{\chi_p}{k_{\mathrm{eff}}4\pi}\int_{0}^{\infty}\nu\Sigma_f(E')\phi(E')dE' }_{\text{Fission}} + \underbrace{ \frac{\chi_d}{4\pi}\sum_{i=1}^I\lambda_iC_i }_{\text{Delayed Neutron}}

- Angular flu $\psi$ is 7-dimensional, scalar flux $\phi$ is five-dimensional
- Energy dependence: large $E$ fast energies, low $E$ thermal energies
- Prompt neutrons appear immediately, delayed neutrons appear after precursor $C_i$ decays
- Nuclear data in the form of cross sections: $v$, $\Sigma_t$, $\Sigma_s$, $\chi_p$, $\nu\Sigma_f$, $\chi_d$, $\lambda_i$, $\beta_i$

!---

# Steady-State Neutron Transport Equation

!equation
\vec{\Omega}\cdot\vec{\nabla}\psi + \Sigma_t\psi = \mathcal{S}\psi + \frac{1}{\textcolor{red}{k}}\frac{\chi}{4\pi}\int_{0}^{\infty}\nu\Sigma_f(E')\phi(E')dE'

- Forms a generalized eigenvalue problem where $k$ is the eigenvalue and $\psi$ is the eigenvector
- Eigenvalue form answers the question “How much more/less fission do we need to sustain a steady state?”
- Largest eigenvalue is the famous multiplication factor: $k_{\mathrm{eff}} = \frac{\text{Neutron production}}{\text{Neutron losses}}$
- Eigenvalue form is a homogeneous equation:

  !equation
  \psi \text{ is a solution} \rightarrow c\psi \text{ is also a solution } (\text{$c$ is a real number})

- Eigenvalue equation does not tell you the power level, user sets it

!---

# Discretizing the Neutron Tranport Equation

- Discretize energy: Multigroup formulation

  - Split energy range in ranges or groups and index by $g$
  - Integrate the transport equation over groups and define “multigroup cross sections”

  !equation
  \psi(E) \rightarrow \psi^g = \int_{E_{g-1}}^{E_g}\psi(E)dE, \qquad \Sigma_t(E)\rightarrow\Sigma^g_t, \qquad g=1,...,G

- Discretize angular direction

  - Discrete set of direction: Discrete ordinate or SN method
  - Function expansion using Spherical Harmonics or PN method
  - Diffusion approximation

- Discretize space (energy/direction set of coupled advection equations)

  - Discontinuous Galerkin FEM (DFEM)
  - Continuous Galerkin FEM with stabilization (CFEM)
  - Hybrid Galerkin FEM (HFEM)

!---

# Diffusion Approximation

### Workhorse for fast multiphysics neutronics

!equation
-\vec{\nabla}\cdot D^g \vec{\nabla} \phi^g + \Sigma_r^g\phi^g = \sum_{g'\neq g}\Sigma_s^{g\leftarrow g'}\phi^{g'} + \frac{\chi^g}{\textcolor{red}{k}}\sum_{g=1}^G\nu\Sigma_f^{g'}\phi^{g'}

- Solving the transport equation is expensive because it is 7-dimensional
- Diffusion equation is an equation for the scalar flux
- When is the diffusion approximation valid?

  - Angular anisotropies are not too strong
  - Within the domain, far away from strong absorbers
  - Neutron current changes slowly with time

- Largely successful because it is used for homogenized geometries

!---

# Homogenization and Equivalence

!row!
!col! width=65%
- Smear detailed geometry over a larger region
- By smearing heterogeneities we make angular flux ”smoother” $\rightarrow$ diffusion theory becomes valid
- Workflow:

  - Steady-state Monte Carlo calculation of detailed geometry $\rightarrow$ compute flux-volume-weighted cross sections
  - Equivalence calculation with super homogenization (SPH)
  - Run transient multiphysics
!col-end!

!col width=35%
!media homogenization.png

!row-end!

- Need for homogenization equivalence

  - Flux-volume weighted cross sections do not preserve all leakages and reaction rates $\rightarrow$ loss of balance (homogenization error)
  - Equivalence ensures proper conservation (mitigates homogenization error)

!---

# Homogenization Example

!row!
!col! width=50%
### Serpent

- Explicit pebble representation
- Explicit TRISO representation per pebble (random)
- Overlay a r-z-𝛉 mesh and homogenize over “spectral zones”

!media homogenization_serpent.png
!col-end!

!col! width=50%
### Griffin

- Homogenized 2-D or 3-D model

!table
| HTR-10 | $k_{\mathrm{eff}}$ | $\Delta$pcm |
| :- | - | - |
| Diffusion | 1.02544 | 2520 |
| P1 | 1.03706 | 3682 |
| P2 | 1.04053 | 4029 |
| SPH-Diff | 1.00230 | 0 |

!media homogenization_griffin.png style=width:70%;margin-left:auto;margin-right:auto;display:block;background-color:white;
!col-end!
!row-end!
