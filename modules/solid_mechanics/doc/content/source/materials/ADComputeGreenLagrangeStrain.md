# ADComputeGreenLagrangeStrain

!syntax description /Materials/ADComputeGreenLagrangeStrain

The Green-Lagrange strain $E$ is a non-linear total strain

\begin{equation}
  E=\frac12 \left(F^TF-1\right)
\end{equation}

Where $F$ is the deformation gradient tensor. It can be combined with
ADComputeLinearElasticStress to compute a second Piola-Kirchhoff stress (use the
ADStressDivergence kernel on the *undisplaced* mesh). This combination is
called a St. Venant-Kirchhoff hyper elasticity model.

!syntax parameters /Materials/ADComputeGreenLagrangeStrain

!syntax inputs /Materials/ADComputeGreenLagrangeStrain

!syntax children /Materials/ADComputeGreenLagrangeStrain

!bibtex bibliography
