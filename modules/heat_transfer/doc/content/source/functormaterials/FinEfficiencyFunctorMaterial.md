# FinEfficiencyFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a fin efficiency,
which can be used with [FinEnhancementFactorFunctorMaterial.md], for example.

## Formulation

Under the following assumptions:

- straight fin,
- uniform cross section, and
- adiabatic tip,

the fin efficiency $\eta_f$ is the following [!citep](incropera2002):

!equation
\eta_f = \frac{\text{tanh}(m L)}{m L} \,,

!equation
m = \sqrt{\frac{\mathcal{H} P}{k A_c}} \,,

where

- $\mathcal{H}$ is the heat transfer coefficient,
- $k$ is the thermal conductivity of the fin,
- $L$ is the fin height, i.e., the distance the fin extends above the surface,
- $P$ is the fin perimeter, and
- $A_c$ is the fin cross-sectional area.

Note that the fin perimeter and cross-sectional area are not needed separately; only the
ratio $P / A_c$ is needed. For example, for a rectangular fin with width $w$ and thickness $t$
and a cylindrical pin fin with diameter $D$:

| Fin type | $P$ | $A_c$ | $\frac{P}{A_c}$ |
| :- | - | - | - |
| Rectangular | $2 w + 2 t$ | $w t$ | $\frac{2 w + 2 t}{w t}$ |
| Cylindrical pin | $\pi D$ | $\frac{\pi D^2}{4}$ | $\frac{4}{D}$ |

## Usage

This functor material creates a functor material property for the fin efficiency
$\eta_f$ with the name given by the parameter
[!param](/FunctorMaterials/FinEfficiencyFunctorMaterial/fin_efficiency_name).

The quantities $\mathcal{H}$, $k$, $L$, and $P / A_c$ are provided by the [functor](/Functors/index.md)
parameters
[!param](/FunctorMaterials/FinEfficiencyFunctorMaterial/heat_transfer_coefficient),
[!param](/FunctorMaterials/FinEfficiencyFunctorMaterial/thermal_conductivity),
[!param](/FunctorMaterials/FinEfficiencyFunctorMaterial/fin_height), and
[!param](/FunctorMaterials/FinEfficiencyFunctorMaterial/fin_perimeter_area_ratio), respectively.

!syntax parameters /FunctorMaterials/FinEfficiencyFunctorMaterial

!syntax inputs /FunctorMaterials/FinEfficiencyFunctorMaterial

!syntax children /FunctorMaterials/FinEfficiencyFunctorMaterial
