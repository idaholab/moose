# NSFVPumpFunctorMaterial

!syntax description /FunctorMaterials/NSFVPumpFunctorMaterial

This is a functor material to set up the effective volume force of a pump. For a forward operating pump, the volume force model reads as follows:

\begin{equation}
  f_V = \frac{\rho |\vec{g}| h(Q) A_r}{V_r} ,
\end{equation}

where:

- $\rho$ is the fluid density
- $|\vec{g}|$ is the norm of the gravity vector
- $A_r$ is the rated area of the pump (typically taken as the area of the upstream volume leading to the pump)
- $V_r$ is the rated volume of the pump (we recommend taking this one as the area of the block leading to the pump)
- $h(Q)$ is the pressure head of the pump in units of {\it length} as a function of the flow rate with units of {\it volume / time}

The scaling of the pressure head of the pump is performed via the non-dimensional rotation speed of the pump defined as follows:

\begin{equation}
  \omega_s = \frac{\omega Q^{\frac{1}{2}}}{\left( g h \right)^{\frac{3}{4}}} ,
\end{equation}

For scaling purposes, the user can provide the following parameters:

- Rated and actual flow rate at the pump. The actual flow rate at the pump must be provided via a post-processor
- Rated and actual rotation speed of the pump. The actual rotation speed of the pump is a controllable parameter

To allow the pump to operate in reverse flow conditions, the user must set `enable_negative_rotation = true`.
The scaling of the model in the negative rotation condition is the same as the positive one.
However, for negative rotation, the user can provide the homologous pressure head curve via the `pressure_head_function_negative_rotation` function.
If the pressure head function for the positive direction should be used for negative rotation, the user should set `symmetric_negative_pressure_head = true`.

!syntax parameters /FunctorMaterials/NSFVPumpFunctorMaterial

!syntax inputs /FunctorMaterials/NSFVPumpFunctorMaterial

!syntax children /FunctorMaterials/NSFVPumpFunctorMaterial
