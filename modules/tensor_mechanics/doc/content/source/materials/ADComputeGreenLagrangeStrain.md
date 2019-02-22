# ADComputeGreenLagrangeStrain

!syntax description /Materials/ADComputeGreenLagrangeStrain<RESIDUAL>

The Green-Lagrange strain $E$ is a non-linear total strain

$$
E=\frac12 \left(F^TF-1\right)
$$

Where $F$ is the deformation gradient tensor. It can be combined with
ADComputeLinearElasticStress to compute a second Piola-Kirchhoff stress (use the
ADStressDivergence kernel on the *undisplaced* mesh). This combination is
called a St. Venant-Kirchhoff hyper elasticity model.

!syntax parameters /Materials/ADComputeGreenLagrangeStrain<RESIDUAL>

!syntax inputs /Materials/ADComputeGreenLagrangeStrain<RESIDUAL>

!syntax children /Materials/ADComputeGreenLagrangeStrain<RESIDUAL>

!bibtex bibliography
