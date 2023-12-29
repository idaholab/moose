# ComputeHypoelasticStVenantKirchhoffStress

!syntax description /Materials/ComputeHypoelasticStVenantKirchhoffStress

## Overview

This class implements the following constitutive model
\begin{equation}
      \hat{\sigma}_{ij} = C_{ijkl} \dot{l}_{kl}
\end{equation}
where $\dot{l}$ is the spatial velocity gradient, and $C$ is the elasticity tensor in the current configuration which is pushed forward
from the reference configuration by
\begin{equation}
    C_{ijkl} = \frac{1}{J} F_{im} F_{jn} F_{kp} F_{lq} C_{nmpq}.
\end{equation}

This model inherits from [`ComputeLagrangianObjectiveStress`](ComputeLagrangianObjectiveStress.md) and so
it will integrate an objective stress rate to provide a "hypoelastic" large deformation constitutive
response based solely on the small strain model.

This model is interesting in the sense that, if the Truesdell objective rate is used, it is equivalent to the [hyperelastic St. Venant-Kirchhoff model](ComputeStVenantKirchhoffStress.md).

!syntax parameters /Materials/ComputeHypoelasticStVenantKirchhoffStress

!syntax inputs /Materials/ComputeHypoelasticStVenantKirchhoffStress

!syntax children /Materials/ComputeHypoelasticStVenantKirchhoffStress
