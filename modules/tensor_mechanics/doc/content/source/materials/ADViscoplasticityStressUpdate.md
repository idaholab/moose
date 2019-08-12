# AD Viscoplasticity Stress Update

!syntax description /Materials/ADViscoplasticityStressUpdate

## Description

`ADViscoplasticityStressUpdate` implements the [Gurson-Tvergaard-Needleman](Gurson:1977gg) (GTN) and
[Leblond-Perrin-Suqeut](Leblond:1994kl) (LPS) following the theory described above.
`ADViscoplasticityStressUpdate` uses similar techniques as
[ADRadialReturnStressUpdate](ADRadialReturnStressUpdate.md) to compute the gauge stress in order to
correctly calculate the plastic strain in a porous material. `ADViscoplasticityStressUpdate` must be
used in conjunction with
[ADComputeMultiplePorousInelasticStress](ADComputeMultiplePorousInelasticStress.md) in order to capture
the porosity evolution.

### Notation

Mean stress: $\boldsymbol{\Sigma}, \Sigma_{ij}$\\
Microscopic stress: $\boldsymbol{\sigma}, \sigma_{ij}$\\
Mean strain: $\mathbf{E}, E_{ij}$\\
Microscopic strain: $\boldsymbol{\epsilon}, \epsilon_{ij}$\\
Hydrostatic stress: $\Sigma_m$\\
Deviatoric stress: $\boldsymbol{\Sigma}^{\prime} = \Sigma_{ij} - \tfrac{1}{3}\delta_{ij}\Sigma_{kk}$\\
Equivalent stress: $\Sigma_{eq} = \sqrt{\tfrac{3}{2}\left(\boldsymbol{\Sigma}^{\prime}:\boldsymbol{\Sigma}^{\prime}\right)}$\\
Trace: $\text{tr}(\mathbf{T})=T_{11} + T_{22} + T_{33}$ \\
Porosity: $f$ \\
Stress exponent: $n$


## Theory

As pores nucleate within a material system, the dissipative potential that governs overall
stress-strain response and drives local void growth mechanisms is enhanced. Simply put, +the
constitutive behavior of a material and the evolution of porosity within are highly coupled
processes+.  In high-temperature and other extreme environments, this potential should include
effects of viscoplasticity and diffusion.

Homogenization of a composite material's plastic potential can be accomplished using the
[!cite](Bishop:1951fb) upper-bound theorem for dissipation, which says that any possible dissipation
field solved at the macroscopic level is an upper bound to the volume average of local dissipation
within a material system.

\begin{equation}
  \boldsymbol{\Sigma}:\dot{\mathbf{E}} \geq \langle \boldsymbol{\sigma}:\boldsymbol{\dot{\epsilon}} \rangle = \int_V \boldsymbol{\sigma}:\boldsymbol{\dot{\epsilon}} \text{d}V.
\end{equation}

In many continuum level modeling problems, the simulation length scale is larger than individual
voids themselves, and therefore methods in homogenization are required to model the voided material
response with an approximated, simplified medium. These methods aim to match the dissipative
potential of the true medium [true_vs_homo], thereby ensuring accurate stress-strain constitutive
behavior of the porous material system.

!media media/tensor_mechanics/true_vs_homo.png
      style=width:80%;margin-left:5%;
      id=true_vs_homo
      caption=Schematic comparing the true sub-material response and the homogenized response and showing that the two methods match energy dissipation.

[!cite](Gurson:1977gg) employed this approach to solve for dissipation in a purely-plastic
(rate-insensitive) material, and derived an exact expression for the velocity and strain fields
within a unit cell based on isotropic expressions of the [!cite](Rice:1969ky) fields. The solution
to these velocity fields was derived using boundary conditions of strain at the surface of the unit
cell, local incompressibility of the material system, and an overall minimization of work to bring
the approximate upper-bound solution as close as possible to the true solution. In Gurson's
formulation, it should be noted that despite incompressibility of the material itself, the overall
unit cell (void + material) is in fact compressible due to the void. Therefore, void growth,
$\dot{f}$ can be modeled simply based on overall dilatation:

\begin{equation}
  \dot{f} = (1-f)\text{tr}(\dot{\mathbf{E}})  
\end{equation}

The exact result for average dissipation in the unit cell was correlated against an analytical
solution so that it would be useful for application in finite element codes for large-scale
component analysis. Several fitting parameters were introduced into Gurson's model by
[!cite](Tvergaard:1984ip) to account for a periodic array of voids (rather than the original unit
cell), leading to the famous GTN model that is the most widely-used damage and porosity evolution
model for ductile materials.

[!cite](Leblond:1994kl) (LPS) extended the GTN model to account for rate-sensitive plastics. While
far less common than the rate-insensitive GTN model, the work by Leblond et al. generated a similar
analytical solution to GTN, but accounted for dissipation in the material, making it also very accessible to
finite element implementation. It is expected that differences between rate-sensitive (i.e. LPS) and
rate-insensitive (i.e. GTN) plasticity will become significant at lower temperatures and stresses. As such, the
LPS model is selected for describing dissipation in a rate-sensitive voided material. In the case of
a single dissipative potential, described by a Norton-type power law:

\begin{equation}
  \psi(\Lambda _n (\boldsymbol{\Sigma},f)) = \frac{\sigma_0 \dot{\epsilon}_0}{n+1} \left(\frac{\Lambda_n(\boldsymbol{\Sigma},f)}{\sigma_0}\right)^{n+1},
  \label{eq:dissipation}
\end{equation}
\begin{equation}
  \dot{\mathbf{E}} = \frac{\partial\psi(\boldsymbol{\Sigma})}{\partial\boldsymbol{\Sigma}} = \frac{\partial\psi(\boldsymbol{\Sigma})}{\partial\Lambda_n} \frac{\partial\Lambda_n}{\partial\boldsymbol{\Sigma}}.
  \label{eq:lps_strain}
\end{equation}
Here, the gauge stress, $\Lambda_n$,  is used to translate applied stress and porosity to strain rate response in a power law creeping material with rate-sensitivity factor $n$, is given via the minimization of the residual $\mathcal{R}$:
\begin{equation}
  \mathcal{R} = \left(\frac{\Sigma_{eq}}{\Lambda_n(\boldsymbol{\Sigma},f)}\right)^2 + f\left[h_n + \frac{n-1}{n+1}\frac{1}{h_n}\right] - 1-\frac{n-1}{n+1}f^2 = 0
\end{equation}
where $h_n$ is a rate sensitivity factor. The law proposed by [!cite](Leblond:1994kl)reduces exactly to the [!cite](Gurson:1977gg) model when
using a rate-insensitive exponent.

## Implementation

The inelastic strain is currently calculated explicitly,
\begin{equation}
\mathbf{E}(t+\Delta t) = \mathbf{E}(t) + \dot{\mathbf{E}}\Delta t,
\end{equation}
where the strain rate is calculated via,
\begin{equation}
  \dot{\mathbf{E}} = \frac{\partial\psi(\boldsymbol{\Sigma})}{\partial\boldsymbol{\Sigma}} = \frac{\partial\psi(\boldsymbol{\Sigma})}{\partial\Lambda_n} \frac{\partial\Lambda_n}{\partial\boldsymbol{\Sigma}}.
  \label{eq:strain_rate}
\end{equation}
By taking the derivative of [eq:strain_rate] with respect to the gauge stress, the familiar Norton power law is formed,
\begin{equation}
  \frac{\partial\psi(\boldsymbol{\Sigma})}{\partial\Lambda_n} = \frac{\dot{\epsilon}_0}{\sigma_0^n} \left(\Lambda_n(\boldsymbol{\Sigma},f)\right)^n,
  \label{eq:norton}
\end{equation}
providing a clear link between traditional power law creep solves; by replacing $\Sigma_{eq}$
utilized in tradiation power law creep equations with a gauge stress, the exact strain dissipation
is captured due to the act of homogenization.

Using the LPS model, the gauge stress is calculated by minimizing the residual,
\begin{equation}
  \mathcal{R} = \left(\frac{\Sigma_{eq}}{\Lambda_n(\boldsymbol{\Sigma},f)}\right)^2 + f\left[h_n + \frac{n-1}{n+1}\frac{1}{h_n}\right] - 1-\frac{n-1}{n+1}f^2 = 0
  \label{eq:residual}
\end{equation}

The rate sensitivity factor $h_n$ is a function of the strain rate stress exponent, gauge stress, and equivalent stress,
\begin{equation}
  h_n = \left(1+\frac{1}{n}\left(s_f\frac{|\Sigma_m|}{\Lambda_n(\boldsymbol{\Sigma},f)}\right)^{\frac{n+1}{n}}\right)^n
\label{eq:h}
\end{equation}
where $s_f$ is a shape factor that depends on the pore shape:
\begin{equation}
s_f =\begin{cases}
  \frac{3}{2} &\text{for spherical pores} \\
  \sqrt{3} &\text{for cylindrical pores}
  \label{eq:shape_factor}
\end{cases}
\end{equation}

In addition, the mean stress utilized in [eq:h] is different depending on the shape of the pore,
\begin{equation}
\Sigma_m =\begin{cases}
  \frac{1}{3}(\Sigma_{1,1} + \Sigma_{2,2} + \Sigma_{3,3})  &\text{for spherical pores} \\
  \frac{1}{2}(\Sigma_{1,1} + \Sigma_{2,2}) &\text{for cylindrical pores}
  \label{eq:mean_stress}
\end{cases}
\end{equation}

The GTN can be solved in exactly the same manner as the LPS model by taking $n\rightarrow \infty$ in [eq:residual] and [eq:shape_factor], reducing $\mathcal{R}$ to,
\begin{equation}
  \mathcal{R} = \left(\frac{\Sigma_{eq}}{\Lambda_{\infty}(\boldsymbol{\Sigma},f)}\right)^2 + 2f \cosh\left(s_f\frac{\Sigma_m}{\Lambda_{\infty}}\right) - 1 - f^2 = 0
\end{equation}

Once a solution for $\Lambda_n$ is found, the strain rate can be determined using [eq:strain_rate] with the relationship,
\begin{equation}
 \frac{\partial\Lambda_n}{\partial\boldsymbol{\Sigma}} = \frac{\Lambda_n \frac{\partial\mathcal{R}}{\partial\boldsymbol{\Sigma}}}{\frac{\partial\mathcal{R}}{\partial\boldsymbol{\Sigma}}:\boldsymbol{\Sigma}}.
\end{equation}

## Example Input Files

In all cases, `ADViscoplasticityStressUpdate` must be combined with
[ADComputeMultiplePorousInelasticStress](ADComputeMultiplePorousInelasticStress.md) in order to
calculate the stress and capture the porosity evolution of the material:

!listing modules/tensor_mechanics/test/tests/ad_viscoplasticity_stress_update/lps_single.i block=Materials

In this case, the power law coefficient defined in [eq:norton] is provided as an example here as a
`ParsedMaterial`. Note, if necessary, the coefficient must be provided as an AD material if
variables or variable dependent materials are utilized to calculate coefficient.

If several different stress exponents are required, separate `ADViscoplasticityStressUpdate` must be
specified, and combined in
[ADComputeMultiplePorousInelasticStress](ADComputeMultiplePorousInelasticStress.md):

Here,  materials calculated by `ADViscoplasticityStressUpdate` are prepended with `base_name` to
separate their contributions to the overall system. Note, this is different than the `base_name`
provided in the [TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md)

!listing modules/tensor_mechanics/test/tests/ad_viscoplasticity_stress_update/lps_dual.i block=Materials

!syntax parameters /Materials/ADViscoplasticityStressUpdate

!syntax inputs /Materials/ADViscoplasticityStressUpdate

!syntax children /Materials/ADViscoplasticityStressUpdate

!bibtex bibliography
