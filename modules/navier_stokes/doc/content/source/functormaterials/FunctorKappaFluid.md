# FunctorKappaFluid

!syntax description /FunctorMaterials/FunctorKappaFluid

## Description

Most macroscale models neglect thermal dispersion [!cite](suikkanen,y_li), in which case $\kappa_f$ is given as

\begin{equation}
\label{eq-KappaFluid}
\kappa_f=\epsilon k_f\ .
\end{equation}

Neglecting thermal dispersion is expected to be a reasonable approximation for high Reynolds numbers [!cite](gunn1987_htc,littman),
but for low Reynolds numbers more sophisticated models should be used [!cite](becker).
Because thermal dispersion acts to increase the diffusive effects, neglecting thermal dispersion is
(thermally) conservative in the sense that peak temperatures are usually higher [!cite](becker).

!syntax parameters /FunctorMaterials/FunctorKappaFluid

!syntax inputs /FunctorMaterials/FunctorKappaFluid

!syntax children /FunctorMaterials/FunctorKappaFluid
