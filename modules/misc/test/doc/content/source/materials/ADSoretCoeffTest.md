# ADSoretCoeffTest

## Description

`ADSoretCoeffTest` is a test AD material property used to test the `ADKernel`
`ADThermoDiffusion`. A AD material property with the name `soret_coefficient` and
of the form,
\begin{equation}
  k = u / T^2
\end{equation}
where $k$ is the soret_coefficient, $T$ is the temperature, and $u$ is a coupled
 variable.

!syntax parameters /ADMaterials/ADSoretCoeffTest<RESIDUAL>

!syntax inputs /ADMaterials/ADSoretCoeffTest<RESIDUAL>

!syntax children /ADMaterials/ADSoretCoeffTest<RESIDUAL>

!bibtex bibliography
