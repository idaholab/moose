# FinEnhancementFactorFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes the enhancement
factor $\zeta$ for a convective heat transfer condition due to extended surfaces such as fins.

## Formulation

Consider the convective heat transfer rate $Q_0$ at a surface with area $A_0$
without the addition of any fins:

!equation
Q_0 = \mathcal{H} (T - T_\infty) A_0 \,,

where

- $\mathcal{H}$ is the heat transfer coefficient,
- $T$ is the surface temperature, and
- $T_\infty$ is the fluid temperature.

The addition of extended surfaces such as fins to the surface enhances this
heat transfer rate by a factor $\zeta$:

!equation id=total_heat_rate
Q = \zeta Q_0 \,.

The fin efficiency $\eta_f$ characterizes the performance of a single fin and
is defined as the ratio of the heat transfer rate through the fin $Q_f$ to the
theoretical maximum heat transfer rate through the fin $Q_{f,\text{max}}$ [!citep](incropera2002):

!equation
\eta_f \equiv \frac{Q_f}{Q_{f,\text{max}}} \,,

!equation
Q_{f,\text{max}} \equiv \mathcal{H} (T - T_\infty) A_{f,\text{single}} \,,

where $A_{f,\text{single}}$ is the surface area of the fin.

The total surface efficiency $\eta_t$ characterizes the performance of an array
of fins on a surface, not just a single fin, as $\eta_f$ does, and is defined
as the ratio of the total heat transfer rate $Q$ of the entire surface to the
theoretical maximum heat transfer rate $Q_{\text{max}}$ [!citep](incropera2002):

!equation id=total_efficiency
\eta_t \equiv \frac{Q}{Q_{\text{max}}} \,,

!equation id=total_efficiency_max
Q_{\text{max}} \equiv \mathcal{H} (T - T_\infty) A_t \,,

Combining [!eqref](total_heat_rate), [!eqref](total_efficiency), and [!eqref](total_efficiency_max)
gives the definition of the enhancement factor:

!equation
\zeta = \eta_t \frac{A_t}{A_0} \,,

where $A_t$ is the total surface area, including fins.
The total efficiency is computed from the fin efficiency and the fin area
ratio $A_f / A_t$ [!citep](incropera2002):

!equation
\eta_t = 1 - (1 - \eta_f) \frac{A_f}{A_t} \,,

where $A_f$ is the surface area of all fins on the surface.

## Usage

This functor material creates a functor material property for the enhancement
factor $\zeta$ with the name given by the parameter
[!param](/FunctorMaterials/FinEnhancementFactorFunctorMaterial/fin_enhancement_factor_name).

The quantities $\eta_f$, $A_f / A_t$, and $A_t / A_0$ are provided by the [functor](/Functors/index.md) parameters
[!param](/FunctorMaterials/FinEnhancementFactorFunctorMaterial/fin_efficiency),
[!param](/FunctorMaterials/FinEnhancementFactorFunctorMaterial/fin_area_fraction), and
[!param](/FunctorMaterials/FinEnhancementFactorFunctorMaterial/area_increase_factor), respectively.

!syntax parameters /FunctorMaterials/FinEnhancementFactorFunctorMaterial

!syntax inputs /FunctorMaterials/FinEnhancementFactorFunctorMaterial

!syntax children /FunctorMaterials/FinEnhancementFactorFunctorMaterial
