# Compute Finite Strain in Cartesian System

!syntax description /Materials/ComputeFiniteStrain

## Description

The finite strain mechanics approach used in the MOOSE tensor_mechanics module is the incremental
corotational form from
[Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract).

In this form, the generic time increment under consideration is such that $t \in [t_n, t_{n+1}]$. The
configurations of the material element under consideration at $t = t_n$ and $t = t_{n+1}$ are denoted
by $\kappa_n$, and $\kappa_{n + 1}$, respectively. The incremental motion over the time increment is
assumed to be given in the form of the inverse of the deformation gradient $\hat{\boldsymbol{F}}$ of
$\kappa_{n + 1}$ with respect to $\kappa_n$, which may be written as

\begin{equation}
\hat{\boldsymbol{F}}^{-1} = 1 - \frac{\partial \hat{\boldsymbol{u}}}{\partial \boldsymbol{x}},
\end{equation}
where $\hat{\boldsymbol{u}}(\boldsymbol{x})$ is the incremental displacement field for the time step, and
$\boldsymbol{x}$ is the position vector of materials points in $\kappa_{n+1}$. Note that
$\hat{\boldsymbol{F}}$ is NOT the deformation gradient, but rather the incremental deformation gradient
of $\kappa_{n+1}$ with respect to $\kappa_n$. Thus, $\hat{\boldsymbol{F}} = \boldsymbol{F}_{n+1}
\boldsymbol{F}_n^{-1}$, where $\boldsymbol{F}_n$ is the total deformation gradient at time $t_n$.

For this form, we assume
\begin{equation}
\begin{aligned}
\dot{\boldsymbol{F}} \boldsymbol{F}^{-1} =& \boldsymbol{D}\ \mathrm{(constant\ and\ symmetric),\ } t_n<t<t_{n+1}\\
\boldsymbol{F}(t^{-}_{n+1}) =& \hat{\boldsymbol{U}}\ \mathrm{(symmetric\ positive\ definite)}\\
\boldsymbol{F}(t_{n+1}) =& \hat{\boldsymbol{R}} \hat{\boldsymbol{U}} = \hat{\boldsymbol{F}}\ (\hat{\boldsymbol{R}}\ \mathrm{proper\ orthogonal})
\end{aligned}
\end{equation}

In tensor mechanics, there are two decomposition options to obtain the strain increment and rotation
increment: TaylorExpansion and EigenSolution, with the default set to TaylorExpansion.  According to
[Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract), the stretching
rate tensor and rotation matrix can be expressed in terms of 'incremental' deformation gradient
$\hat{\boldsymbol{F}}$ as
\begin{equation}
\boldsymbol{D} = \frac{1}{\Delta t}\log({\hat{\boldsymbol{C}}^{1/2}}) = \frac{1}{\Delta t}\log({(\hat{\boldsymbol{F}}^{T} \hat{\boldsymbol{F}})^{1/2}})
\end{equation}
and

\begin{equation}
\hat{ \boldsymbol{R} } = \hat{\boldsymbol{F}} \hat{\boldsymbol{U}}^{-1}
\end{equation}

## Taylor Expansion

According to [Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract), the
stretching rate tensor $\boldsymbol{D}$ and rotation matrix $\boldsymbol{R}$ can be approximated using Taylor
expansion as:
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
\sin^{2} \theta^{a} =& Q \\
\frac{\sin \theta^{a}}{2\sqrt{Q}} =& \frac{1}{2}\left[ \frac{PQ(3-Q)+P^{3}+Q^{2}}{(P+Q)^{3}} \right]^{1/2}\\
\frac{1-\cos \theta^{a}}{4Q} =& \frac{1}{8} + Q\frac{P^{2}-12(P-1)}{32P^2} + Q^{2}\frac{(P-2)(P^{2}-10P+32)}{64P^3}\\
 +& Q^{3}\frac{1104-992P+376P^{2}-72P^{3}+5P^{4}}{512P^{4}}\\
\cos^{2} \theta^{a} =& P + \frac{3P^{2}[1-(P+Q)]}{(P+Q)^{2}} - \frac{2P^{3}[1-(P+Q)]}{(P+Q)^{3}}\\
P =& \frac{1}{4}(tr(\hat{\boldsymbol{F}}^{-1}) - 1)^{2}
\end{aligned}
\end{equation}
The sign of $\cos \theta^{a}$ is set by examining the sign of $(tr(\hat{\boldsymbol{F}}^{-1}) - 1)$.

## Eigen-Solution

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

## Volumetric Locking Correction

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
The [TensorMechanics Master Action](/systems/Modules/TensorMechanics/Master/index.md) is designed to
automatically determine and set the strain and stress divergence parameters correctly for the
selected strain formulation.  We recommend that users employ the
[TensorMechanics Master Action](/systems/Modules/TensorMechanics/Master/index.md) whenever possible
to ensure consistency between the test function gradients and the strain formulation selected.

Although not recommended, it is possible to directly use the `ComputeFiniteStrain` material
in the input file.

!listing modules/tensor_mechanics/test/tests/volumetric_deform_grad/elastic_stress.i
         block=Materials/strain

!syntax parameters /Materials/ComputeFiniteStrain

!syntax inputs /Materials/ComputeFiniteStrain

!syntax children /Materials/ComputeFiniteStrain
