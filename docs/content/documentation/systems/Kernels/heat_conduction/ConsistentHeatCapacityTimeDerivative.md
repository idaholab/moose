# ConsistentHeatCapacityTimeDerivative
!syntax description /Kernels/ConsistentHeatCapacityTimeDerivative

This kernel is similar to HeatCapacityConductionTimeDerivative. The heat capacity $C_p = \rho c_p$ is
expected as material property. The difference to HeatCapacityConductionTimeDerivative is that
the derivative of $C_p$ with respect to temperature is included:
\begin{equation}
  \frac{\partial C_p T}{\partial t} = \left[C_p +T \frac{\partial C_p}{\partial T} \right] \frac{\partial T}{\partial t}
\end{equation}
This makes this kernel consistent where HeatCapacityConductionTimeDerivative is not. It is, however, only conservative as $\Delta t \rightarrow 0$. Consistency was shown via Method of Manufactured
Solution test.

!syntax parameters /Kernels/ConsistentHeatCapacityTimeDerivative

!syntax inputs /Kernels/ConsistentHeatCapacityTimeDerivative

!syntax children /Kernels/ConsistentHeatCapacityTimeDerivative
