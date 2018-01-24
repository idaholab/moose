# ConsistentSpecificHeatTimeDerivative
!syntax description /Kernels/ConsistentSpecificHeatTimeDerivative

This kernel is similar to SpecificHeatConductionTimeDerivative. The specific heat capacity $c_p$ and density $\rho$ are
expected as material properties. The difference to SpecificHeatConductionTimeDerivative is that
the derivatives of $c_p$ and $\rho$ with respect to temperature are included:
\begin{equation}
  \frac{\partial c_p \rho T}{\partial t} = \left[c_p \rho +T c_p \frac{\partial \rho}{\partial T} + T \rho \frac{\partial c_p}{\partial T}\right] \frac{\partial T}{\partial t}
\end{equation}
This makes this kernel consistent where SpecificHeatConductionTimeDerivative is not. It is, however, only conservative as $\Delta t \rightarrow 0$.
Consistency was shown via Method of Manufactured Solution test.

!syntax parameters /Kernels/ConsistentSpecificHeatTimeDerivative

!syntax inputs /Kernels/ConsistentSpecificHeatTimeDerivative

!syntax children /Kernels/ConsistentSpecificHeatTimeDerivative
