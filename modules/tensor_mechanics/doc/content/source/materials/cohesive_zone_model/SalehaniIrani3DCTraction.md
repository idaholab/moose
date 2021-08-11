# SalehaniIrani 3D Coupled Traction separation law

!syntax description /Materials/SalehaniIrani3DCTraction

## Description

This class implements the non-stateful traction separation law proposed by [!cite](salehani2018coupled).
This material model is an extension to 3 dimensions of the 2-dimensional traction-separation law proposed by Xu and Needleman [!cite](xu1993void).
This traction separation law should only be used for monotonic loading conditions as it will not produce realistic results for unloading and reloading.
This model can be used for 1D, 2D and 3D problems.

The traction separation relationship is defined by:
\begin{equation}
T_i = \frac{\phi_i}{\delta_i}\frac{\Delta_i}{\delta_i} \exp [ -\sum_{j=1}^{d}(\frac{\Delta_j}{\delta_j})^{\alpha}]
\end{equation}
where $i$ and $j$ are indices representing the displacement jump component with the index 1 being associated with the opening direction, $\alpha$ is a model parameter with values
 $\alpha = 1$ if $j==1$ or $\alpha = 2$ if $j == 2,3$, $d$ is a parameter representing the number of dimensions of the problem, $\Delta_i$ is the current gap value and $\delta_i$ is the characteristic length of separation related to the maximum sustainable traction.
The symbol $\phi_i$ represents the work of separation and is defined as
\begin{equation}
\phi_i = T_{i,max} \lambda \delta_i
\end{equation}
where $\lambda = e$ if $i==1$ or $\lambda = \sqrt{2 e}$ if $i == 2,3$. The parameter $T_{i,max}$ represents the maximum allowed traction that the interface can withstand in the $i-th$ direction. Note that the values of maximum allowed traction can be different in the normal and tangential directions, however $T_{2,max}$ is assumed to be equal to $T_{3,max}$. The same restrictions on $T_{i,max}$ apply to $\delta_i$

## Examples

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/czm_multiple_action_and_materials.i block=Materials/czm_3dc
!syntax parameters /Materials/SalehaniIrani3DCTraction
!syntax inputs /Materials/SalehaniIrani3DCTraction
!syntax children /Materials/SalehaniIrani3DCTraction
