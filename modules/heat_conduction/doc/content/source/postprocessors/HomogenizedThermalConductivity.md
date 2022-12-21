# HomogenizedThermalConductivity

!syntax description /Postprocessors/HomogenizedThermalConductivity

!alert note title=Einstein summation convention
Einstein summation convention is used in this documentation page.

## Description

This `PostProcessor` computes

!equation id=eq:0
\lambda^\text{H}_{ij}=\frac{1}{\left|\text{Y}\right|}\int_\text{Y}\left(\lambda_{ij} + \lambda_{ik} \frac{\partial \chi^j}{\partial y_k}\right)\;\text{d}\bm{y},

where $\lambda^\text{H}_{ij}$ is the $i,j$-th element of the homogenized thermal conductivity tensor, $\lambda_{ij}$ is the $i,j$-th element of the thermal conductivity tensor in the heterogeneous problem, $\chi^j$ is the j-th characteristic function defined as:

!equation id=eq:1
\int_Y \frac{\partial v}{\partial y_i}\lambda_{ij} \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_i}\lambda_{ik} \text{d}y


This `PostProcessor` is used in conjunction with the [Heat Conduction](HeatConduction.md) `Kernel` and the [Homogenized Heat Conduction](HomogenizedHeatConduction.md) `Kernel`.
An application can be found in [!cite](hales15homogenization).

!alert note title=Notation
First, compared to standard notation for homogenization theory applied to thermal conductivity (e.g. [!cite](SONG2006710)),
MOOSE computes $-\chi^j$. This is achieved by inverting the sign of the right hand side of [eq:1]. This leads to an inversion of the sign in the parenthesis in [eq:0].
Second, [eq:1] is the weak form of the equation typically provided in literature and integration by parts
on the right hand side leads to another sign flip.
Third, in contrast to [!cite](hales15homogenization) some notational inconsistencies are resolved in this document.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D.i block=Postprocessors/k_xx


!syntax parameters /Postprocessors/HomogenizedThermalConductivity

!syntax inputs /Postprocessors/HomogenizedThermalConductivity

!syntax children /Postprocessors/HomogenizedThermalConductivity

!bibtex bibliography
