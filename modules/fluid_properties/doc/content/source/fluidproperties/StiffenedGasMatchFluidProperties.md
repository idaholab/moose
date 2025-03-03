# StiffenedGasMatchFluidProperties

This class derives from [StiffenedGasFluidPropertiesBase.md] and determines
the stiffened gas parameters from a reference [SinglePhaseFluidProperties.md]
object, evaluated at a given pressure and temperature: $(p_\text{ref}, T_\text{ref})$:

!equation
c_p = c_{p,\text{ref}}(p_\text{ref}, T_\text{ref}) \,,

!equation
c_v = c_{v,\text{ref}}(p_\text{ref}, T_\text{ref}) \,,

!equation
\gamma = \frac{c_p}{c_v} \,,

!equation
M = M_\text{ref} \,,

!equation
q = h_\text{ref}(p_\text{ref}, T_\text{ref}) - \gamma c_v T_\text{ref} \,,

!equation
p_\infty = \rho_\text{ref}(p_\text{ref}, T_\text{ref}) (\gamma - 1) c_v T_\text{ref} - p_\text{ref} \,,

!equation
q' = s_\text{ref}(p_\text{ref}, T_\text{ref}) - c_v \text{ln}\left(
  \frac{T_\text{ref}^\gamma}{(p_\text{ref} + p_\infty)^{\gamma - 1}}\right) \,,

!equation
\mu = \mu_\text{ref}(p_\text{ref}, T_\text{ref}) \,,

!equation
k = k_\text{ref}(p_\text{ref}, T_\text{ref}) \,,

!equation
T_\text{crit} = T_\text{crit,ref} \,,

!equation
\rho_\text{crit} = \rho_\text{crit,ref} \,,

!equation
e_\text{crit} = e_\text{crit,ref} \,.

The critical properties $T_\text{crit}$, $\rho_\text{crit}$, and $e_\text{crit}$
are optional since it is not uncommon for the reference fluid properties
object to have these interfaces unimplemented; by default, these will return
not-a-numbers. If the critical values are required for your application,
[!param](/FluidProperties/StiffenedGasMatchFluidProperties/require_critical_properties)
may be set to `true` to evaluate them.

!syntax parameters /FluidProperties/StiffenedGasMatchFluidProperties

!syntax inputs /FluidProperties/StiffenedGasMatchFluidProperties

!syntax children /FluidProperties/StiffenedGasMatchFluidProperties

!bibtex bibliography
