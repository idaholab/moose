#ComputeFiniteStrain

!syntax description /Materials/ComputeFiniteStrain

##Description
The finite strain mechanics approach used in the MOOSE tensor_mechanics module is the incremental corotational form from [Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract).

In this form, the generic time increment under consideration is such that $t \in [t_n, t_{n+1}]$. The configurations of the material element under consideration at $t = t_n$ and $t = t_{n+1}$ are denoted by $\kappa_n$, and $\kappa_{n + 1}$, respectively. The incremental motion over the time increment is assumed to be given in the form of the inverse of the deformation gradient $\hat{\mathbf{F}}$ of $\kappa_{n + 1}$ with respect to $\kappa_n$, which may be written as

$$
\hat{\mathbf{F}}^{-1} = 1 - \frac{\partial \hat{\mathbf{u}}}{\partial \mathbf{x}},
$$

where $\hat{\mathbf{u}}(\mathbf{x})$ is the incremental displacement field for the time step, and $\mathbf{x}$ is the position vector of materials points in $\kappa_{n+1}$. Note that $\hat{\mathbf{F}}$ is NOT the deformation gradient, but rather the incremental deformation gradient of $\kappa_{n+1}$ with respect to $\kappa_n$. Thus, $\hat{\mathbf{F}} = \mathbf{F}_{n+1} \mathbf{F}_n^{-1}$, where $\mathbf{F}_n$ is the total deformation gradient at time $t_n$.

For this form, we assume
$$
\begin{eqnarray}
\dot{\mathbf{F}} \mathbf{F}^{-1} &=& \mathbf{D}\ \mathrm{(constant\ and\ symmetric),\ } t_n<t<t_{n+1}\\
\mathbf{F}(t^{-}_{n+1}) &=& \hat{\mathbf{U}}\ \mathrm{(symmetric\ positive\ definite)}\\
\mathbf{F}(t_{n+1}) &=& \hat{\mathbf{R}} \hat{\mathbf{U}} = \hat{\mathbf{F}}\ (\hat{\mathbf{R}}\ \mathrm{proper\ orthogonal})
\end{eqnarray}
$$

In tensor mechanics, there are two decomposition options to obtain the strain increment and rotation increment: TaylorExpansion and EigenSolution, with the default set to TaylorExpansion.
According to [Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract), the stretching rate tensor and rotation matrix can be expressed in terms of 'incremental' deformation gradient $\hat{\mathbf{F}}$ as
$$
\mathbf{D} = \frac{1}{\Delta t}\log({\hat{\mathbf{C}}^{1/2}}) = \frac{1}{\Delta t}\log({(\hat{\mathbf{F}}^{T} \hat{\mathbf{F}})^{1/2}})
$$
and
$$
\hat{\mathbf{R} = \hat{\mathbf{F}} \hat{\mathbf{U}}^{-1}
$$

###TaylorExpansion:

According to [Rashid 1993](http://onlinelibrary.wiley.com/doi/10.1002/nme.1620362302/abstract), the stretching rate tensor $\mathbf{D}$ and rotation matrix $\mathbf{R}$ can be approximated using Taylor expansion as:
the approximated stretching rate tensor
$$
\mathbf{D}^{a} = \frac{1}{\Delta t}\{ -\frac{1}{2}(\hat{\mathbf{C}}^{-1} - \mathbf{I}) + \frac{1}{4}(\hat{\mathbf{C}}^{-1} - \mathbf{I})^{2} - \frac{1}{6}(\hat{\mathbf{C}}^{-1} - \mathbf{I})^{3} + ... \}
$$
the approximated rotation matrix
$$
\hat{R}_{ij}^{a} = \delta_{ij}\cos \theta^{a} + \frac{1-\cos \theta^{a}}{4Q} \alpha_{i}\alpha_{j} - \frac{\sin \theta^{a}}{2\sqrt{Q}}\epsilon_{ijk}\alpha_{k}
$$
with
$$
\begin{eqnarray}
\sin^{2} \theta^{a} &=& Q \\
\frac{\sin \theta^{a}}{2\sqrt{Q}} &=& \frac{1}{2}\left[ \frac{PQ(3-Q)+P^{3}+Q^{2}}{(P+Q)^{3}} \right]^{1/2}\\
\frac{1-\cos \theta^{a}}{4Q} &=& \frac{1}{8} + Q\frac{P^{2}-12(P-1)}{32P^2} + Q^{2}\frac{(P-2)(P^{2}-10P+32)}{64P^3}\\ &+& Q^{3}\frac{1104-992P+376P^{2}-72P^{3}+5P^{4}}{512P^{4}}\\
\cos^{2} \theta^{a} &=& P + \frac{3P^{2}[1-(P+Q)]}{(P+Q)^{2}} - \frac{2P^{3}[1-(P+Q)]}{(P+Q)^{3}}\\
P &=& \frac{1}{4}(tr(\hat{\mathbf{F}}^{-1}) - 1)^{2}
\end{eqnarray}
$$
The sign of $\cos \theta^{a}$ is set by examining the sign of $(tr(\hat{\mathbf{F}}^{-1}) - 1)$.

###EigenSolution:

The stretching rate tensor can be calculated by the eigenvalues $\lambda$ and eigenvectors $\mathbf{v}$ of $\hat{\mathbf{C}}$.
$$
\mathbf{D} = \log{\sqrt{\lambda_{1}}}\mathbf{N}_{1} + \log{\sqrt{\lambda_{2}}}\mathbf{N}_{2} + \log{\sqrt{\lambda_{3}}}\mathbf{N}_{3}
$$
with $\lambda$ being the eigenvalue and $\mathbf{N}$ matrix being constructed from the corresponding eigenvector.
$$
\mathbf{N}_{i} = \mathbf{v}_{i}\mathbf{v}_{i}^{T}
$$
the 'incremental' stretching tensor
$$
\hat{\mathbf{U}} = \sqrt{\lambda_{1}}\mathbf{N}_{1} + \sqrt{\lambda_{2}}\mathbf{N}_{2} + \sqrt{\lambda_{3}}\mathbf{N}_{3}
$$
and thus
$$
\hat{\mathbf{R}} = \hat{\mathbf{F}} \hat{\mathbf{U}}^{-1}
$$

In `ComputeFiniteStrain`, $\hat{\mathbf{F}}$ is calculated in the computeStrain method, including a volumetric locking correction of
$$
\hat{\mathbf{F}}_{corr} = \hat{\mathbf{F}} \left( \frac{|\mathrm{av}_{el}(\hat{\mathbf{F}})|}{|\hat{\mathbf{F}}|} \right)^{\frac{1}{3}},
$$
where $\mathrm{av}_{el}()$ is the average value for the entire element. The strain increment and the rotation increment are calculated in `computeQpStrain()`. Once the strain increment is calculated, it is added to the total strain from $t_n$. The total strain from $t_{n+1}$ must then be rotated using the rotation increment.


!syntax parameters /Materials/ComputeFiniteStrain

!syntax inputs /Materials/ComputeFiniteStrain

!listing modules/tensor_mechanics/tests/finite_strain_elastic/finite_strain_elastic_new_test.i start=strain end=stress

!syntax children /Materials/ComputeFiniteStrain
