# CZM 3DC traction separation law

!syntax description /Materials/CZM3DCLaw

##Description

This class implements the non-stateful traction separation law proposed by [!cite](salehani2018coupled). This traction separation law is optimal for monotonic loading conditions because damage is not incorporated. This model can be used for both 1D, 2D and 3D problems.

The traction separation relationship is the following:

\begin{equation}
T_i = \frac{\phi_i}{\delta_i}\frac{\Delta_i}{\delta_i} \exp [ -\sum_{j=1}^{d}(\frac{\Delta_j}{\delta_j})^{\alpha}]
\end{equation}

where $i$ and $j$ are indexes representing the displacement jumps component with the indexes 1 being associated with the opening direction, $\alpha$ is a model parameter with values
 $\alpha = 1$ if $j==1$ or $\alpha = 2$ if $j == 2,3$, $d$ is a parameter representing the number of dimension of the problem, $\Delta_i$ is the current gap value and $\delta_i$ are characteristic length of separations related to the maximum sustainable traction.

The symbol $\phi_i$ represents the work of separation and is defined as

\begin{equation}
\phi_i = T_{i,max} \lambda \delta_i
\end{equation}

where $\lambda = e$ if $i==1$ or  $\lambda = \sqrt{2 e}$  if $i == 2,3$. The parameter  T_{i,max} represents the maximum allowed traction that the interface can withstand in the $i-th$ direction. Note that the values of maximum allowed traction can be different in normal and tangential direction, however $T_{2,max}$ is assumed to be equal to $T_{3,max}$. The same restrictions of $T_{i,max}$  apply to $\delta_i$


## examples

!listing modules/tensor_mechanics/test/tests/CZM/czm_3DC_load_complex.i block=Materials/czm_3dc


!syntax parameters /Materials/CZM3DCLaw

!syntax inputs /Materials/CZM3DCLaw

!syntax children /Materials/CZM3DCLaw
