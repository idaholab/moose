# ADComputeGreenLagrangeStrain

!syntax description /ADMaterials/ADComputeGreenLagrangeStrain<RESIDUAL>

The Green-Lagrange strain $E$ is a non-linear total strain

$$
E=\frac12 \left(F^TF-1\right)
$$

Where $F$ is the deformation gradient tensor. It can be combined with
ADComputeLinearElasticStress to compute a second Piola-Kirchhoff stress (use teh
ADStressDivergence kernel on the *undisplaced* mesh). This combinations is
called a St. Venant-Kirchhoff hyper elasticity model.

!syntax parameters /ADMaterials/ADComputeGreenLagrangeStrain<RESIDUAL>

!syntax inputs /ADMaterials/ADComputeGreenLagrangeStrain<RESIDUAL>

!syntax children /ADMaterials/ADComputeGreenLagrangeStrain<RESIDUAL>

!bibtex bibliography
