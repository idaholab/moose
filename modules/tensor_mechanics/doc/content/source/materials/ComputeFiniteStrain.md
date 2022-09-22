# Compute Finite Strain in Cartesian System

!syntax description /Materials/ComputeFiniteStrain

## Description

This class is used to compute the strain increment, total strain, and incremental rotation for finite strain problems. The finite strain approach used is the incremental
corotational form [!citep](rashid1993incremental). This approach computes logarithmic strains and strain increments.

### Incremental Configurations

In this form, the generic time increment under consideration is such that
\begin{equation}
  \label{eqn:time_notation}
  t \in [t_n, t_{n+1}]
\end{equation}
The configurations of the material element under consideration at $t = t_n$ and
$t = t_{n+1}$ are denoted by $\kappa_n$, and $\kappa_{n + 1}$, respectively for
the previous and the current incremental configurations.

### Deformation Gradient Definition

The deformation gradient represents the change in a material element from the
reference configuration to the current configuration [!citep](malvern1969introduction).
In the incremental formulation used in the `ComputeFiniteStrain` class, the incremental
deformation gradient represents the change in the material element from the previous
configuration, $\kappa_n$, to the current configuration, $\kappa_{n+1}$.
Mathematically this relationship is given as
\begin{equation}
  \hat{\boldsymbol{F}} = \frac{\partial{\boldsymbol{x}_{n+1}}}{\partial{\boldsymbol{x}_n}},
\end{equation}
where $\boldsymbol{x}_{n+1}$ is the position vector of materials points in $\kappa_{n+1}$,
and $\boldsymbol{x}_{n}$ is the position vector of materials points in $\kappa_{n}$.

!alert! note title=Incremental vs Total Deformation Gradient
Note that $\hat{\boldsymbol{F}}$ is NOT the deformation gradient, but rather the incremental deformation gradient
of $\kappa_{n+1}$ with respect to $\kappa_n$. Thus $\hat{\boldsymbol{F}} = \boldsymbol{F}_{n+1}
\boldsymbol{F}_n^{-1}$, where $\boldsymbol{F}_n$ is the total deformation gradient at time $t_n$.
!alert-end!

Following the explanation of this procedure given by [!cite](zhang2018modified),
the incremental deformation gradient can be multiplicatively decomposed into an
incremental rotation tensor, $\boldsymbol{\hat{R}}$, and the incremental right
stretch tensor, $\boldsymbol{\hat{U}}$
\begin{equation}
  \label{eqn:polar_decomposition_deform_grad}
  \boldsymbol{\hat{F}} = \boldsymbol{\hat{R}} \cdot \boldsymbol{\hat{U}}
\end{equation}
where $\boldsymbol{\hat{R}}$ is a proper orthogonal rotation tensor and the stretch
tensor, $\boldsymbol{\hat{U}}$, is symmetric and positive definite.
The incremental right Cauchy-Green deformation tensor, $\boldsymbol{\hat{C}}$,
can be given in terms of $\boldsymbol{\hat{U}}$ by subsituting
[eqn:polar_decomposition_deform_grad] into the definition for $\boldsymbol{\hat{C}}$
from [!cite](malvern1969introduction):
\begin{equation}
  \label{eqn:right_green_cauchy_deformation_tensor}
  \boldsymbol{\hat{C}} = \boldsymbol{\hat{F}}^T \cdot \boldsymbol{\hat{F}} = \boldsymbol{\hat{U}}^T \cdot \boldsymbol{\hat{R}}^T \cdot \boldsymbol{\hat{R}} \cdot \boldsymbol{\hat{U}} = \boldsymbol{\hat{U}}^2
\end{equation}
where the orthogonal nature of $\boldsymbol{\hat{R}}$ enables the simplification
given above. Thus $\boldsymbol{\hat{U}}$ can be computed from $\boldsymbol{\hat{C}}$ as
\begin{equation}
  \label{eqn:definition_stretch_tensor}
  \boldsymbol{\hat{U}} = \boldsymbol{\hat{C}}^\frac{1}{2}
\end{equation}
which can be evaluated by performing a spectral decomposition of $\boldsymbol{\hat{C}}$.
Once $\boldsymbol{\hat{U}}$ has been computed, the multiplicative decomposition
of the deformation gradient is used to find the incremental rotation tensor
$\boldsymbol{\hat{R}}$ and the stretching rate $\boldsymbol{D}$.
Following [!cite](rashid1993incremental), the stretching rate tensor can be expressed in terms
of the 'incremental' right Cauchy-Green deformation tensor
\begin{equation}
  \label{eqn:stretching_tensor_definition}
  \boldsymbol{D} = \frac{1}{\Delta t}\log({\hat{\boldsymbol{C}}^{1/2}})
\end{equation}

This incremental streteching rate tensor can then be used as the work conjugate
for a stress measure, or used to compute another strain
measure. The most computationally expensive part of this procedure is the spectral
decomposition of $\boldsymbol{\hat{C}}$ to find $\boldsymbol{\hat{U}}$. This
decomposition can be computed exactly using an [Eigensolution](#eigensolution),
yet an approximation of this can be computed with much lower computational expense
using a [Taylor expansion](#taylorexpansion) procedure.
This class provides options to perform this calculation either way, and the
[Taylor expansion](#taylorexpansion) is the default.

This class also provides an additional option for
computing the stretching rate tensor $\boldsymbol{D}$ and incremental rotation tensor $\boldsymbol{\hat{R}}$
using the method of [!cite](hughes1980finite). This method involves evaluating the displacement field at the midpoint of the timestep, and as a
result, requires updating stress and strain measures at the old configuration rather than the current.

#### Taylor Expansion id=taylorexpansion

The stretching rate tensor $\boldsymbol{D}$ and incremental rotation matrix $\hat{\boldsymbol{R}}$
can be approximated using Taylor expansion as [!cite](rashid1993incremental):
the approximated stretching rate tensor
\begin{equation}
\boldsymbol{D}^{a} = \frac{1}{\Delta t}\left[ -\frac{1}{2}(\hat{\boldsymbol{C}}^{-1} - \boldsymbol{I}) + \frac{1}{4}(\hat{\boldsymbol{C}}^{-1} - \boldsymbol{I})^{2} - \frac{1}{6}(\hat{\boldsymbol{C}}^{-1} - \boldsymbol{I})^{3} + ... \right]
\end{equation}
the approximated rotation matrix
\begin{equation}
\hat{R}_{ij}^{a} = \delta_{ij}\cos \theta^{a} + \frac{1-\cos \theta^{a}}{4Q} \alpha_{i}\alpha_{j} - \frac{\sin \theta^{a}}{2\sqrt{Q}}\epsilon_{ijk}\alpha_{k}
\end{equation}
with
\begin{equation}
\begin{aligned}
\sin^{2} \theta^{a} =& Q \\[1.5em]
\frac{\sin \theta^{a}}{2\sqrt{Q}} =& \frac{1}{2}\left[ \frac{PQ(3-Q)+P^{3}+Q^{2}}{(P+Q)^{3}} \right]^{1/2}\\[1.5em]
\frac{1-\cos \theta^{a}}{4Q} =& \frac{1}{8} + Q\frac{P^{2}-12(P-1)}{32P^2} + Q^{2}\frac{(P-2)(P^{2}-10P+32)}{64P^3}\\[1.5em]
 +& Q^{3}\frac{1104-992P+376P^{2}-72P^{3}+5P^{4}}{512P^{4}}\\[1.5em]
\cos^{2} \theta^{a} =& P + \frac{3P^{2}[1-(P+Q)]}{(P+Q)^{2}} - \frac{2P^{3}[1-(P+Q)]}{(P+Q)^{3}}\\[1.5em]
P =& \frac{1}{4}(tr(\hat{\boldsymbol{F}}^{-1}) - 1)^{2}
\end{aligned}
\end{equation}
The sign of $\cos \theta^{a}$ is set by examining the sign of $(tr(\hat{\boldsymbol{F}}^{-1}) - 1)$.

#### Eigen-Solution id=eigensolution

The stretching rate tensor can be calculated by the eigenvalues $\lambda$ and eigenvectors
$\boldsymbol{v}$ of $\hat{\boldsymbol{C}}$.
\begin{equation}
\boldsymbol{D} = \log{\sqrt{\lambda_{1}}}\boldsymbol{N}_{1} + \log{\sqrt{\lambda_{2}}}\boldsymbol{N}_{2} + \log{\sqrt{\lambda_{3}}}\boldsymbol{N}_{3}
\end{equation}
with $\lambda$ being the eigenvalue and $\boldsymbol{N}$ matrix being constructed from the corresponding
eigenvector.
\begin{equation}
\boldsymbol{N}_{i} = \boldsymbol{v}_{i}\boldsymbol{v}_{i}^{T}
\end{equation}
the 'incremental' stretching tensor
\begin{equation}
\hat{\boldsymbol{U}} = \sqrt{\lambda_{1}}\boldsymbol{N}_{1} + \sqrt{\lambda_{2}}\boldsymbol{N}_{2} + \sqrt{\lambda_{3}}\boldsymbol{N}_{3}
\end{equation}
and thus
\begin{equation}
\hat{\boldsymbol{R}} = \hat{\boldsymbol{F}} \hat{\boldsymbol{U}}^{-1}
\end{equation}

#### Hughes-Winget Approximation id=hugheswinget

As also described in [!cite](rashid1993incremental), the stretching rate tensor $\boldsymbol{D}$ and incremental
rotation matrix $\hat{\boldsymbol{R}}$ in for the Hughes-Winget method are based on the spatial gradient $\boldsymbol{G}$
of the displacement field evaluated at the mid-point of the time step
\begin{equation}
\boldsymbol{G} = 2\left( \hat{\boldsymbol{F}} - \boldsymbol{I}\right) \left( \hat{\boldsymbol{F}} + \boldsymbol{I}\right)^{-1}
\end{equation}
The approximate stretching rate tensor can then be computed as
\begin{equation}
\boldsymbol{D} = \frac{1}{2 \Delta t}\left(\boldsymbol{G} + \boldsymbol{G}^{T} \right)
\end{equation}
and the incremental rotation matrix is approximated by
\begin{equation}
\hat{\boldsymbol{R}} = \left(\boldsymbol{I} + \frac{1}{2}\omega \right) \left(\boldsymbol{I} - \frac{1}{2}\omega \right)^{-1}
\end{equation}
where $\omega = \frac{1}{2}\left(\boldsymbol{G} - \boldsymbol{G}^{T} \right)$.

### Volumetric Locking Correction

In `ComputeFiniteStrain`, $\hat{\boldsymbol{F}}$ is calculated in the computeStrain method, including a
volumetric locking correction of
\begin{equation}
\hat{\boldsymbol{F}}_{corr} = \hat{\boldsymbol{F}} \left( \frac{|\mathrm{av}_{el}(\hat{\boldsymbol{F}})|}{|\hat{\boldsymbol{F}}|} \right)^{\frac{1}{3}},
\end{equation}
where $\mathrm{av}_{el}()$ is the average value for the entire element. The strain increment and the
rotation increment are calculated in `computeQpStrain()`. Once the strain increment is calculated, it
is added to the total strain from $t_n$. The total strain from $t_{n+1}$ must then be rotated using
the rotation increment.

## Example Input File Syntax

The finite strain calculator can be activated in the input file through the use of the
TensorMechanics Master Action, as shown below.

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Modules/TensorMechanics

!alert note title=Use of the Tensor Mechanics Master Action Recommended
The [TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the strain and stress divergence parameters correctly for the
selected strain formulation.  We recommend that users employ the
[TensorMechanics Master Action](/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

Although not recommended, it is possible to directly use the `ComputeFiniteStrain` material
in the input file.

!listing modules/tensor_mechanics/test/tests/volumetric_deform_grad/elastic_stress.i
         block=Materials/strain

When directly using `ComputeFiniteStrain` in an input file as shown above, the
[StressDivergenceTensors](/StressDivergenceTensors.md) kernel must be modified
from the default by setting the parameter `use_displaced_mesh = true`. This setting
is required to maintain consistency in the test function gradients and the
strain formulation. For a complete discussion of the stress diveregence kernel
settings and the corresponding strain classes, see the section on
[Consistency Between Stress and Strain](/tensor_mechanics/StressDivergence.md#consistency_stress_strain_use_displaced_mesh)
in the TensorMechanics module overview. In addition, be aware of the loading
cycle limitations while using finite strains as outlined in the section
[Large Strain Closed Loop Loading Cycle](/tensor_mechanics/Strains.md#large_strain_closed_loop_loading_cycle).


!syntax parameters /Materials/ComputeFiniteStrain

!syntax inputs /Materials/ComputeFiniteStrain

!syntax children /Materials/ComputeFiniteStrain

!bibtex bibliography
